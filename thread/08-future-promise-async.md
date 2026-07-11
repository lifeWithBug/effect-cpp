# C++ 多线程笔记 08：异步任务 (future/promise/async)

## 1. 为什么需要异步任务？

手动管理线程的三个痛点：
- **传返回值麻烦**：通过引用传参、用 atomic、或额外的事件通知
- **异常处理困难**：线程中抛异常，主线程不知道怎么捕获
- **生命周期管理**：线程什么时候结束？结果什么时候就绪？

`std::async` + `std::future` 解决了这些问题。

## 2. std::async——最简单的异步

```cpp
#include <iostream>
#include <future>
#include <thread>

int slow_compute(int n) {
    std::this_thread::sleep_for(std::chrono::seconds(2));
    return n * n;
}

int main() {
    // 异步启动计算——在后台线程中执行
    std::future<int> result = std::async(std::launch::async, slow_compute, 42);

    std::cout << "Main thread is free to do other work...\n";

    // 需要结果时，get() 会阻塞直到结果就绪
    std::cout << "Result: " << result.get() << "\n";  // 阻塞约 2 秒
    return 0;
}
```

关键点：
- `std::async` 返回一个 `std::future` 对象
- `future.get()` 会阻塞直到异步操作完成
- `future.get()` 只能调用一次——之后 future 不再 valid

## 3. 启动策略

```cpp
// 强制新线程执行
std::future<int> f1 = std::async(std::launch::async, func);

// 惰性求值：调用 get() 时才在同一线程执行
std::future<int> f2 = std::async(std::launch::deferred, func);

// 默认策略：由实现决定
std::future<int> f3 = std::async(func);  // 等价于 async | deferred
```

| 策略 | 行为 | 适用场景 |
|------|------|---------|
| `async` | 立即在新线程执行 | 需要并行，结果稍后取 |
| `deferred` | 调用 get() 时才在当前线程执行 | 计算结果可能用不到 |
| 默认 | 由实现决定（通常 async） | 大部分场景 |

**注意**：默认策略下，如果你不调用 `get()` 或 `wait()`，`~future()` 析构时会**阻塞等待直到任务完成**（因为 `async` 启动的关联线程需要 join）。为了避免意外，明确传 `std::launch::async` 是一个好习惯。

## 4. future 的方法

```cpp
std::future<int> f = std::async(std::launch::async, slow_compute, 42);

f.wait();          // 等待任务完成，但不取结果
// ... 确认完成后可以做其他事 ...

int x = f.get();   // 获取结果，只能调用一次

f.valid();         // 检查 future 是否关联了共享状态
// get() 之后 valid() 返回 false
```

## 5. 异常传递

异步函数抛出的异常会保存在 `future` 中，`get()` 时重新抛出：

```cpp
auto f = std::async(std::launch::async, [] {
    throw std::runtime_error("Something went wrong");
    return 42;
});

try {
    int x = f.get();  // 重新抛出异常
} catch (const std::runtime_error& e) {
    std::cout << "Caught: " << e.what() << "\n";
}
```

## 6. std::promise——手动管理异步结果

`std::async` 太"自动化"了。有时候你需要更细粒度的控制——比如线程跑在某个线程池里，或者你想在某个回调中填入结果。

```cpp
#include <future>
#include <thread>
#include <iostream>

void compute_sum(std::promise<int> prom, int a, int b) {
    try {
        int result = a + b;
        prom.set_value(result);     // 设置结果，唤醒 get() 的线程
    } catch (...) {
        prom.set_exception(std::current_exception());  // 传递异常
    }
}

int main() {
    std::promise<int> prom;
    std::future<int> fut = prom.get_future();

    std::thread t(compute_sum, std::move(prom), 10, 20);
    // prom 我已经用完了，move 给了线程

    std::cout << "Waiting for result...\n";
    int result = fut.get();  // 阻塞直到 set_value() 被调用
    std::cout << "Result: " << result << "\n";

    t.join();
    return 0;
}
```

### promise 的注意事项

- promise 是 **move-only** 的，不能复制
- `set_value()` 只能调用一次，再次调用会抛 `std::future_error`
- `get_future()` 也只能调用一次
- 如果 promise 析构了还没 `set_value()`，关联的 future 会收到 `std::future_errc::broken_promise`

## 7. std::packaged_task——包装可调用对象

`packaged_task` 把任意可调用对象包装成一个能自动返回 future 的任务：

```cpp
#include <future>
#include <iostream>
#include <thread>

int compute(int a, int b) {
    return a * b;
}

int main() {
    // 包装函数
    std::packaged_task<int(int, int)> task(compute);
    std::future<int> fut = task.get_future();

    // task 可以在某个线程中执行
    std::thread t(std::move(task), 6, 7);
    t.join();

    std::cout << fut.get() << "\n";  // 42
    return 0;
}
```

### 典型用法：任务队列 + packaged_task

```cpp
class ThreadPool {
    std::vector<std::thread> workers;
    std::queue<std::packaged_task<void()>> tasks;
    std::mutex mtx;
    std::condition_variable cv;
    bool stop = false;
public:
    template<typename F>
    auto enqueue(F&& f) -> std::future<decltype(f())> {
        using ReturnType = decltype(f());
        auto task = std::make_shared<std::packaged_task<ReturnType()>>(
            std::forward<F>(f)
        );
        std::future<ReturnType> res = task->get_future();
        {
            std::lock_guard<std::mutex> lock(mtx);
            tasks.emplace([task]() { (*task)(); });
        }
        cv.notify_one();
        return res;
    }
    // 省略构造函数和析构函数...
};
```

## 8. std::shared_future——多线程共享结果

`std::future` 是 move-only，只能有一个线程调用 `get()`。`shared_future` 可以多个线程同时 `get()`：

```cpp
void parallel_worker(std::shared_future<int> sf) {
    try {
        int result = sf.get();  // 阻塞直到结果就绪
        std::cout << "Got result: " << result << "\n";
    } catch (...) {
        std::cout << "Got exception\n";
    }
}

int main() {
    std::promise<int> prom;
    std::shared_future<int> sf = prom.get_future().share();
    // 现在 sf 是 shared_future，可以被复制

    std::thread t1(parallel_worker, sf);
    std::thread t2(parallel_worker, sf);
    std::thread t3(parallel_worker, sf);

    prom.set_value(42);

    t1.join(); t2.join(); t3.join();
    return 0;
}
```

## 9. 对比总结

| 工具 | 适用场景 | 控制级别 |
|------|---------|---------|
| `std::async` | 简单的异步调用，"发起即忘然后拿结果" | 粗糙 |
| `std::promise` + `std::future` | 需要手动设置结果或传递异常 | 精细 |
| `std::packaged_task` + `std::future` | 把已有函数包装成异步任务单元 | 中等 |
| `std::shared_future` | 多个线程同时等待同一个异步结果 | 共享 |