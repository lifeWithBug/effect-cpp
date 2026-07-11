# C++ 多线程笔记 09：线程安全的设计模式

## 1. 线程池 (ThreadPool)

### 为什么需要线程池？
每次 `new thread` 都涉及系统调用（内核态切换）和栈分配（默认约 1~8 MB）。频繁创建销毁线程的开销很大。线程池的思路：

- 启动时创建固定数量的线程
- 任务被推入一个队列
- 空闲线程从队列取任务执行
- 不需要重复创建线程

### 一个完整的线程池实现

```cpp
#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>

class ThreadPool {
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex mtx_;
    std::condition_variable cv_;
    bool stop_{false};

public:
    explicit ThreadPool(size_t n)
        : workers_(n) {
        for (size_t i = 0; i < n; ++i) {
            workers_[i] = std::thread([this] {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(mtx_);
                        // 等待：有任务可做，或收到停止信号
                        cv_.wait(lock, [this] {
                            return stop_ || !tasks_.empty();
                        });
                        if (stop_ && tasks_.empty()) {
                            return;  // 退出线程
                        }
                        task = std::move(tasks_.front());
                        tasks_.pop();
                    }  // 解锁——任务在锁外执行
                    task();
                }
            });
        }
    }

    // 提交任务，返回 future 获取结果
    template<typename F, typename... Args>
    auto enqueue(F&& f, Args&&... args)
        -> std::future<decltype(f(args...))> {

        using ReturnType = decltype(f(args...));
        auto task = std::make_shared<std::packaged_task<ReturnType()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        std::future<ReturnType> res = task->get_future();
        {
            std::lock_guard<std::mutex> lock(mtx_);
            if (stop_) {
                throw std::runtime_error("enqueue on stopped ThreadPool");
            }
            tasks_.emplace([task]() { (*task)(); });
        }
        cv_.notify_one();
        return res;
    }

    ~ThreadPool() {
        {
            std::lock_guard<std::mutex> lock(mtx_);
            stop_ = true;
        }
        cv_.notify_all();          // 唤醒所有线程检查 stop_
        for (auto& worker : workers_) {
            worker.join();
        }
    }
};
```

### 使用示例

```cpp
ThreadPool pool(4);  // 4 个线程

// 提交无返回值任务
pool.enqueue([] {
    std::cout << "Task running in thread pool\n";
});

// 提交有返回值任务
auto future = pool.enqueue([](int a, int b) {
    return a + b;
}, 10, 20);
std::cout << "Result: " << future.get() << "\n";  // 30
```

## 2. 读写锁 (std::shared_mutex)

### 读多写少的场景

读操作不修改数据，可以安全地并发。写操作必须独占。用普通 mutex 无法区分读和写——读读都会互相阻塞。

```cpp
#include <shared_mutex>

class Database {
    std::map<std::string, std::string> data_;
    mutable std::shared_mutex mtx_;  // C++17
public:
    // 读操作：共享锁，不阻塞其他读者
    std::string get(const std::string& key) const {
        std::shared_lock<std::shared_mutex> lock(mtx_);
        auto it = data_.find(key);
        return it != data_.end() ? it->second : "";
    }

    // 写操作：独占锁，阻塞一切读者和写者
    void set(const std::string& key, const std::string& val) {
        std::unique_lock<std::shared_mutex> lock(mtx_);
        data_[key] = val;
    }
};
```

| 锁类型 | 底层 mutex | 语义 | 能否并发 |
|--------|-----------|------|---------|
| `std::shared_lock` | `std::shared_mutex` | 共享锁 | 多个 shared_lock 可并发 |
| `std::unique_lock` | `std::shared_mutex` | 独占锁 | 同一时间只能一个 |

**注意**：读写锁比普通 mutex 有额外开销（需要维护读者计数）。只有读操作远多于写操作时才有收益。

## 3. 一次性初始化 (std::call_once)

有些资源只需要初始化一次：全局单例、日志系统、缓存预热。

### 简陋的做法：双检锁 + volatile

```cpp
// ❌ C++11 之前的老写法，依赖 volatile 和双检锁
volatile bool initialized = false;
std::mutex mtx;

void init() {
    if (!initialized) {
        std::lock_guard<std::mutex> lock(mtx);
        if (!initialized) {
            // 初始化...
            initialized = true;
        }
    }
}
```

### C++11 推荐的写法

```cpp
std::once_flag flag;

void init_logger() {
    // 只会执行一次——即使多个线程同时调用 std::call_once
    std::cout << "Logger initialized\n";
}

void log(const std::string& msg) {
    std::call_once(flag, init_logger);
    std::cout << "[LOG] " << msg << "\n";
}

int main() {
    std::thread t1([]{ log("hello"); });
    std::thread t2([]{ log("world"); });
    t1.join(); t2.join();
    // 输出：
    // Logger initialized
    // [LOG] hello
    // [LOG] world
    return 0;
}
```

`std::once_flag` 不可复制不可移动。多个 `std::call_once` 调用同一个 flag 时，只有一个会执行初始化函数。

### 线程安全的局部 static 变量

其实 C++11 保证局部 static 变量的初始化是线程安全的，很多时候根本不需要显式的 call_once：

```cpp
class Logger {
    static Logger& instance() {
        static Logger logger;  // C++11 起：线程安全
        return logger;
    }
};
```

这种方式在大部分情况下已经够用。`std::call_once` 更适合初始化多个相关资源的场景。

## 4. 线程局部存储 (thread_local)

### 基本概念

每个线程拥有自己独立的变量副本。修改不会影响其他线程，不需要同步。

```cpp
#include <iostream>
#include <thread>

thread_local int counter = 0;

void worker(const std::string& name) {
    for (int i = 0; i < 3; ++i) {
        ++counter;
        std::cout << "[" << name << "] counter = " << counter << "\n";
    }
}

int main() {
    std::thread t1(worker, "A");
    std::thread t2(worker, "B");
    t1.join();
    t2.join();
    // 输出（可能交错）：
    // [A] counter = 1
    // [A] counter = 2
    // [A] counter = 3
    // [B] counter = 1
    // [B] counter = 2
    // [B] counter = 3
    // 两者互不影响
    return 0;
}
```

### thread_local 的存储位置
- 每个线程的 thread_local 变量在**线程私有存储区**，不在共享的堆/栈上
- 线程结束时，所有 thread_local 变量的析构函数被调用

### 实际应用场景

**场景 1：每线程随机数种子**
```cpp
thread_local std::mt19937 rng(std::random_device{}());

int get_random() {
    std::uniform_int_distribution<int> dist(0, 99);
    return dist(rng);
}
```

**场景 2：每线程日志缓冲区**
```cpp
thread_local std::string log_buffer;

void log_message(const std::string& msg) {
    log_buffer += msg + "\n";
    if (log_buffer.size() > 4096) {
        flush_to_disk(log_buffer);
        log_buffer.clear();
    }
}
```

**场景 3：每线程内存池（减少锁争用）**
```cpp
thread_local std::vector<int> thread_cache;

void process(int data) {
    thread_cache.push_back(data);
    if (thread_cache.size() >= 1000) {
        // 批量处理，减少全局锁的争用
        global_process(thread_cache);
        thread_cache.clear();
    }
}
```

## 5. RAII 与线程安全

### 用 RAII 管理线程资源

```cpp
class ThreadGuard {
    std::thread& t_;
public:
    explicit ThreadGuard(std::thread& t) : t_(t) {}
    ~ThreadGuard() {
        if (t_.joinable()) t_.join();
    }
    ThreadGuard(const ThreadGuard&) = delete;
    ThreadGuard& operator=(const ThreadGuard&) = delete;
};
```

### 用 RAII 管理锁

```cpp
// lock_guard / unique_lock / shared_lock 都是 RAII 锁
// 关键原则：临界区退出时自动释放锁
```

### 用 RAII 管理共享资源的访问模式

```cpp
template<typename T>
class LockedPtr {
    T* ptr_;
    std::lock_guard<std::mutex> lock_;
public:
    LockedPtr(T* ptr, std::mutex& mtx)
        : ptr_(ptr), lock_(mtx) {}
    T& operator*() { return *ptr_; }
    T* operator->() { return ptr_; }
};
```