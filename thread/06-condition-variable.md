# C++ 多线程笔记 06：条件变量 (condition_variable)

## 1. 解决的问题

互斥锁解决了"互斥"问题，但现实还有一个常见需求：**等待某个条件成立**。

如果让一个线程不断轮询检查条件：

```cpp
// ❌ 忙等待，浪费 CPU
std::lock_guard<std::mutex> lock(mtx);
while (queue.empty()) {
    lock.unlock();   // 解锁让别人能放数据
    std::this_thread::sleep_for(std::chrono::milliseconds(1));  // 低效
    lock.lock();     // 再锁上检查
}
```

这种方式要么浪费 CPU（短 sleep），要么延迟高（长 sleep）。条件变量就是解决这个问题的。

## 2. 生产者-消费者完整示例

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>

std::queue<int> q;
std::mutex mtx;
std::condition_variable cv;

void producer() {
    for (int i = 0; i < 10; ++i) {
        {
            std::lock_guard<std::mutex> lock(mtx);
            q.push(i);
            std::cout << "Produced: " << i << "\n";
        }  // 先解锁再 notify，避免唤醒的线程立即阻塞在锁上
        cv.notify_one();  // 唤醒消费者
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void consumer() {
    for (int i = 0; i < 10; ++i) {
        std::unique_lock<std::mutex> lock(mtx);
        // wait 做什么？
        // 1. 检查谓词 !q.empty()，如果是 true 直接返回（继续）
        // 2. 如果 false：释放锁，线程进入等待状态
        // 3. 被 notify 唤醒后：重新获取锁，再检查谓词
        // 4. 循环直到谓词为 true
        cv.wait(lock, [] { return !q.empty(); });

        int val = q.front();
        q.pop();
        std::cout << "Consumed: " << val << "\n";
    }
}

int main() {
    std::thread t1(producer);
    std::thread t2(consumer);
    t1.join();
    t2.join();
    return 0;
}
```

## 3. wait() 的内部机制

```cpp
cv.wait(lock, predicate);
```

等价于：

```cpp
while (!predicate()) {
    // 1. 原子地释放锁并进入等待状态
    // 2. 挂起线程，不会消耗 CPU
    // 3. 被 notify 唤醒后，原子地重新获取锁
    // 4. 检查 predicate()
}
```

**必须使用带谓词的 `wait` 重载**。原因有二：

### 原因 1：虚假唤醒 (Spurious Wakeup)
即使没有 `notify_one()`/`notify_all()` 调用，`wait()` 也可能返回。这是操作系统层面的现象，被称为虚假唤醒。

```cpp
// ❌ 错误：裸 wait，可能被虚假唤醒
cv.wait(lock);
// 假设条件成立了？不一定！

// ✅ 正确：用谓词保证条件确实成立
cv.wait(lock, [] { return !queue.empty(); });
// 能走到这里，queue 一定不空
```

### 原因 2：来自 notify_all 的意外唤醒
如果有多个消费者在等待，`notify_all()` 会唤醒全部，但只有第一个能拿到数据：

```
消费者 1: wait 返回 → 检查 !q.empty() → true → pop
消费者 2: wait 返回 → 检查 !q.empty() → false → 继续等
```

带谓词的 wait 自动处理了这种情况。

## 4. Condition Variable 只接受 unique_lock

```cpp
std::condition_variable cv;

// ✅ 正确：unique_lock
std::unique_lock<std::mutex> lock(mtx);
cv.wait(lock, predicate);

// ❌ 编译错误：lock_guard 不支持
std::lock_guard<std::mutex> lock(mtx);
cv.wait(lock, predicate);  // 编译错误
```

为什么？因为 `wait()` 需要在等待时**释放锁**、唤醒时**重新获取锁**——`lock_guard` 不支持这种操作，它只能构造时 lock、析构时 unlock，中间不能干预。

## 5. notify_one vs notify_all

```cpp
cv.notify_one();   // 唤醒一个等待的线程
cv.notify_all();   // 唤醒所有等待的线程
```

| 场景 | 推荐 |
|------|------|
| 单个生产者、单个消费者 | `notify_one()` — 效率高 |
| 单个生产者、多个消费者（每个取一个任务） | `notify_one()` — 只唤醒一个去处理 |
| 多个生产者、多个消费者（条件变化影响所有线程） | `notify_all()` |
| 析构/停止时（所有线程都需要检查退出标志） | `notify_all()` |

## 6. 先解锁再 notify

```cpp
// ✅ 正确做法
{
    std::lock_guard<std::mutex> lock(mtx);
    q.push(val);
}  // 先解锁
cv.notify_one();  // 再唤醒

// ❌ 不太好的做法
std::lock_guard<std::mutex> lock(mtx);
q.push(val);
cv.notify_one();  // 被唤醒的线程发现锁还被自己拿着，只能阻塞
```

`notify_one()` 可能立即调度消费者。如果消费者醒来后第一件事是抢锁，而生产者还持有锁，消费者会再次阻塞——多了一次无意义的上下文切换。

## 7. 线程安全的队列封装

```cpp
#include <queue>
#include <mutex>
#include <condition_variable>

template<typename T>
class ThreadSafeQueue {
    std::queue<T> queue_;
    mutable std::mutex mtx_;
    std::condition_variable cv_;
public:
    // 生产者调用 push
    void push(T value) {
        {
            std::lock_guard<std::mutex> lock(mtx_);
            queue_.push(std::move(value));
        }
        cv_.notify_one();
    }

    // 消费者调用 pop，阻塞等待
    T pop() {
        std::unique_lock<std::mutex> lock(mtx_);
        cv_.wait(lock, [this] { return !queue_.empty(); });
        T val = std::move(queue_.front());
        queue_.pop();
        return val;
    }

    // 带超时的 pop
    bool try_pop(T& value, std::chrono::milliseconds timeout) {
        std::unique_lock<std::mutex> lock(mtx_);
        if (!cv_.wait_for(lock, timeout, [this] { return !queue_.empty(); })) {
            return false;  // 超时
        }
        value = std::move(queue_.front());
        queue_.pop();
        return true;
    }
};
```

## 8. 只触发一次的等待

```cpp
std::mutex mtx;
std::condition_variable cv;
bool event_ready = false;

void waiter() {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [&] { return event_ready; });
    std::cout << "Event received!\n";
}

void notifier() {
    {
        std::lock_guard<std::mutex> lock(mtx);
        event_ready = true;
        // 这里还可以初始化其他数据
    }
    cv.notify_one();
}
```

## 9. wait_for 和 wait_until

```cpp
std::unique_lock<std::mutex> lock(mtx);

// 等待最多 1 秒
if (cv.wait_for(lock, std::chrono::seconds(1),
                [&] { return data_ready; })) {
    process(data);  // 条件在超时前成立
} else {
    std::cout << "Timeout waiting for data\n";
}

// 等待到某个绝对时间点
if (cv.wait_until(lock, std::chrono::steady_clock::now() + std::chrono::seconds(1),
                  [&] { return data_ready; })) {
    process(data);
}
```