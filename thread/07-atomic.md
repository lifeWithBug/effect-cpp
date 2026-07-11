# C++ 多线程笔记 07：原子操作 (std::atomic)

## 1. 为什么需要 atomic？

有些操作太"简单"了，用 mutex 感觉太重了。比如：

```cpp
std::atomic<int> counter{0};

void worker() {
    for (int i = 0; i < 100000; ++i) {
        counter++;  // 原子自增，不用锁
    }
}
```

用 mutex 保护一个 `int` 的自增，涉及系统调用或用户态的锁竞争，开销远比 `atomic` 大。对于计数器、标志位这样的简单变量，`std::atomic` 是更好的选择。

## 2. atomic 支持的常见类型

```cpp
std::atomic<bool> flag{false};
std::atomic<int> counter{0};
std::atomic<unsigned long long> big_counter{0};
std::atomic<double> value{0.0};
std::atomic<int*> ptr{nullptr};

// 自定义类型（需要 TriviallyCopyable）
struct Point { int x, y; };
std::atomic<Point> p;
```

但不是所有类型都能做成 atomic。要求是 trivially copyable（memcpy 可安全拷贝的类型）。通常标准库对 `int`、`long long`、`double` 等基础类型有专门的优化（lock-free）。

## 3. 核心操作

```cpp
std::atomic<int> a{42};

a.store(10);            // 原子写：a = 10
int x = a.load();       // 原子读：x = a
x = a.exchange(20);     // 交换：x = a 的旧值(10)，a = 20
// 此时 x == 10, a == 20
```

### 常用的算术操作

```cpp
std::atomic<int> a{0};

a++;                    // 原子自增
a--;                    // 原子自减
a += 5;                 // 原子加法
a -= 3;                 // 原子减法

int x = a.fetch_add(5); // x = a 的旧值，然后 a += 5
int y = a.fetch_sub(3); // y = a 的旧值，然后 a -= 3
```

`fetch_add` 和 `a += 5` 的语义不同：
- `a += 5` — 返回**新值**
- `a.fetch_add(5)` — 返回**旧值**

### 比较并交换 (CAS)

```cpp
std::atomic<int> a{10};

int expected = 10;
bool success = a.compare_exchange_weak(expected, 20);
// 如果 a == 10:  a = 20, 返回 true
// 如果 a != 10:  expected = a 的当前值, 返回 false

// 用法：用 CAS 实现无锁更新
void atomic_multiply_by_2(std::atomic<int>& val) {
    int expected = val.load();
    while (!val.compare_exchange_weak(expected, expected * 2)) {
        // expected 已被更新为最新值，重试
    }
}
```

`compare_exchange_weak` 和 `compare_exchange_strong`：
- `weak`：在某些平台上可能有**虚假失败**（即使值相等），所以需要 while 循环
- `strong`：保证不会虚假失败，但在某些平台上比 weak 慢

## 4. atomic 能解决所有并发问题吗？不能。

这是一个经典错误：

```cpp
std::atomic<int> balance{100};

void withdraw(int amount) {
    if (balance > amount) {       // 读
        balance -= amount;        // 写
    }
}
```

两个线程同时执行：
```
线程 1: balance > 50 → true       (balance = 100)
线程 2: balance > 50 → true       (balance = 100)
线程 1: balance = 100 - 50 = 50
线程 2: balance = 50 - 50 = 0
```

两个 `withdraw(50)` 都成功了，余额从 100 变成了 0——虽然每个原子操作是安全的，但"判断-修改"这两步**整体不是原子的**。

### 正确做法 1：用 mutex

```cpp
int balance{100};
std::mutex mtx;

bool withdraw(int amount) {
    std::lock_guard<std::mutex> lock(mtx);
    if (balance >= amount) {
        balance -= amount;
        return true;
    }
    return false;
}
```

### 正确做法 2：用 CAS loop

```cpp
std::atomic<int> balance{100};

bool withdraw(int amount) {
    int current = balance.load();
    while (current >= amount) {
        if (balance.compare_exchange_weak(current, current - amount)) {
            return true;  // 成功
        }
        // 失败：current 已被更新为最新余额，重新尝试
    }
    return false;  // 余额不足
}
```

## 5. 常见应用场景

### 场景 1：停止标志

```cpp
std::atomic<bool> stop{false};

void worker() {
    while (!stop) {
        // 持续工作
    }
}

void stop_worker() {
    stop = true;  // 原子写，安全地从外部通知
}
```

### 场景 2：引用计数

```cpp
template<typename T>
class SharedPtr {
    T* ptr_;
    std::atomic<int>* ref_count_;
public:
    explicit SharedPtr(T* p) : ptr_(p), ref_count_(new std::atomic<int>(1)) {}

    SharedPtr(const SharedPtr& other)
        : ptr_(other.ptr_), ref_count_(other.ref_count_) {
        ref_count_->fetch_add(1);  // 原子增加引用计数
    }

    ~SharedPtr() {
        if (ref_count_->fetch_sub(1) == 1) {  // 原子减少，如果是最后一个
            delete ptr_;
            delete ref_count_;
        }
    }
};
```

### 场景 3：延迟初始化（用 double-checked locking）

```cpp
class Logger {
    static std::atomic<Logger*> instance_{nullptr};
    // ...
public:
    static Logger& get() {
        Logger* p = instance_.load(std::memory_order_acquire);
        if (!p) {
            static std::mutex mtx;
            std::lock_guard<std::mutex> lock(mtx);
            p = instance_.load(std::memory_order_relaxed);
            if (!p) {
                p = new Logger();
                instance_.store(p, std::memory_order_release);
            }
        }
        return *p;
    }
};
```

不过 C++11 之后局部 static 变量已经是线程安全的，通常直接写：
```cpp
static Logger& get() {
    static Logger instance;  // C++11 保证线程安全的初始化
    return instance;
}
```

## 6. atomic vs mutex 对比

| 场景 | 推荐 | 原因 |
|------|------|------|
| 计数器（统计、打点） | atomic | 轻量，无锁竞争 |
| 标志位（停止、就绪） | atomic | 简单读写 |
| 多变量联合操作（转账） | mutex | 需要事务语义 |
| 复杂数据结构（队列、map） | mutex | CAS loop 难写且易错 |
| 自定义类型更新 | 看情况 | atomic 要求 TriviallyCopyable |