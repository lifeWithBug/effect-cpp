# C++ 多线程笔记 05：死锁

## 1. 什么是死锁？

两个线程互相等待对方释放资源，导致永久阻塞：

```
线程 A:  持有 m1  → 等待 m2
线程 B:  持有 m2  → 等待 m1
```

两者都没法继续，这就是死锁。

## 2. 代码演示

```cpp
#include <mutex>
#include <thread>
#include <iostream>

std::mutex m1, m2;

void task_a() {
    std::lock_guard<std::mutex> lock1(m1);
    std::cout << "task_a: locked m1\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    std::lock_guard<std::mutex> lock2(m2);  // 等 m2，但被 task_b 持有
    std::cout << "task_a: locked m2\n";
}

void task_b() {
    std::lock_guard<std::mutex> lock2(m2);  // 先锁 m2
    std::cout << "task_b: locked m2\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    std::lock_guard<std::mutex> lock1(m1);  // 等 m1，但被 task_a 持有
    std::cout << "task_b: locked m1\n";
}

int main() {
    std::thread t1(task_a);
    std::thread t2(task_b);
    t1.join();  // 可能永远等在这里
    t2.join();
}
```

## 3. 死锁产生的四个必要条件

| 条件 | 说明 | 能不能破坏？ |
|------|------|-------------|
| **互斥** | 资源一次只能一个线程使用 | 有些资源天生互斥，无法破坏 |
| **持有并等待** | 拿着一把锁还去拿另一把 | 可以一次性锁所有资源 |
| **不可剥夺** | 锁只能由持有者释放 | 可以用 try_lock，放弃就不再等 |
| **循环等待** | A 等 B，B 等 A | **最容易破坏**：固定加锁顺序 |

## 4. 预防方法一：固定加锁顺序（最常用）

```cpp
// 约定：所有代码都按 m1 → m2 → m3 的顺序加锁

void task_a() {
    std::lock_guard<std::mutex> lock1(m1);  // 1
    std::lock_guard<std::mutex> lock2(m2);  // 2
    // 工作...
}

void task_b() {
    std::lock_guard<std::mutex> lock1(m1);  // 1，和 task_a 一致
    std::lock_guard<std::mutex> lock2(m2);  // 2
    // 工作...
}
```

多线程里，**加锁顺序必须全局统一**——这是最基本也最有效的防死锁手段。

**问题**：如果 m1 和 m2 不是同一个类型的锁，或者需要根据参数决定加锁顺序怎么办？

### std::lock() 按参数地址排序
```cpp
void transfer(BankAccount& from, BankAccount& to, int amount) {
    // std::lock 保证按地址排序加锁，避免死锁
    std::lock(from.mutex(), to.mutex());

    // 锁已经拿到了，用 adopt_lock 表明"锁已持有，别再加了"
    std::lock_guard<std::mutex> lock1(from.mutex(), std::adopt_lock);
    std::lock_guard<std::mutex> lock2(to.mutex(), std::adopt_lock);

    from.balance -= amount;
    to.balance += amount;
}
```

## 5. 预防方法二：std::scoped_lock (C++17)

C++17 引入了 `scoped_lock`，可以一次性锁多个 mutex，和 `lock_guard` 类似但更简洁：

```cpp
// C++11/14 写法
void f() {
    std::lock(m1, m2);
    std::lock_guard<std::mutex> l1(m1, std::adopt_lock);
    std::lock_guard<std::mutex> l2(m2, std::adopt_lock);
    // ...
}

// C++17 写法
void f() {
    std::scoped_lock lock(m1, m2);  // 一行搞定，自动死锁避免
    // ...
}
```

```cpp
// 转账例子用 scoped_lock
void transfer(BankAccount& from, BankAccount& to, int amount) {
    std::scoped_lock lock(from.mutex(), to.mutex());
    // 同时锁住两个账户，安全操作
    from.balance -= amount;
    to.balance += amount;
}
```

## 6. 预防方法三：层次锁

给锁分配层级，只能从高往低加锁：

```cpp
class HierarchicalMutex {
    const unsigned long level_;
    unsigned long previous_level_;
    static thread_local unsigned long this_thread_level_;
public:
    explicit HierarchicalMutex(unsigned long level) : level_(level), previous_level_(0) {}

    void lock() {
        if (this_thread_level_ <= level_) {
            throw std::logic_error("mutex hierarchy violated");
        }
        previous_level_ = this_thread_level_;
        this_thread_level_ = level_;
        // 真正 lock 底层 mutex...
    }

    void unlock() {
        this_thread_level_ = previous_level_;
        // 真正 unlock...
    }
};

HierarchicalMutex high(10000);
HierarchicalMutex low(5000);

void f() {
    std::lock_guard<HierarchicalMutex> l1(high);  // OK: level 10000
    std::lock_guard<HierarchicalMutex> l2(low);   // OK: 10000 > 5000
}

void g() {
    std::lock_guard<HierarchicalMutex> l1(low);    // level 5000
    std::lock_guard<HierarchicalMutex> l2(high);   // ❌ 异常！5000 ≤ 10000
}
```

## 7. std::unique_lock 配合 std::try_lock

如果不想等太久，可以用 try_lock：

```cpp
void task_with_timeout() {
    std::unique_lock<std::mutex> lock1(m1, std::defer_lock);
    std::unique_lock<std::mutex> lock2(m2, std::defer_lock);

    // 尝试同时锁两个，最多等 100ms
    for (int i = 0; i < 10; ++i) {
        if (std::try_lock(m1, m2) == -1) {  // -1 表示全部锁成功
            lock1.release();  // 告知 unique_lock 不再管理 m1
            lock2.release();
            // 临界区
            m1.unlock();
            m2.unlock();
            return;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    // 超时处理
    throw std::runtime_error("could not acquire locks");
}
```

## 8. 总结

| 方法 | 适用场景 | 代价 |
|------|---------|------|
| **固定加锁顺序** | 锁的类型固定且数量少 | 需要全局约定 |
| **std::scoped_lock** | 同时锁多把锁 | C++17 可用 |
| **std::lock** | 同时锁多把锁 | 需要 adopt_lock 辅助 |
| **层次锁** | 大型系统，需要强制约束 | 实现复杂 |
| **try_lock** | 不愿意无限等待 | 需要处理失败逻辑 |