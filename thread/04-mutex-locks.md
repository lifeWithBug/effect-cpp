# C++ 多线程笔记 04：互斥量 (mutex) 与锁管理

## 1. std::mutex 基础 mutual exclusion

互斥量（mutex = MUTual EXclusion）是最基础的同步工具：

```cpp
#include <mutex>

std::mutex mtx;     // 全局互斥量

void critical_section() {
    mtx.lock();      // 请求锁。如果锁被其他线程持有，则阻塞等待
    // ███ 临界区开始 ███
    // 同一时间只有一个线程能执行到这里
    // ███ 临界区结束 ███
    mtx.unlock();    // 释放锁，唤醒等待的线程
}
```

### lock() 行为
- 如果锁空闲：立即获得锁，继续执行
- 如果锁被其他线程持有：阻塞，进入等待队列
- 如果当前线程已经持有锁：**死锁**（std::mutex 不可重入）

### try_lock()——非阻塞尝试
```cpp
std::mutex mtx;
if (mtx.try_lock()) {
    // 成功获得锁
    // 临界区...
    mtx.unlock();
} else {
    // 未获得锁，干别的事
    std::cout << "锁被占用了，先做其他事情\n";
}
```

## 2. 裸 lock/unlock 的问题

```cpp
void bad_example() {
    mtx.lock();
    // ... 代码抛异常了！
    // unlock() 永远不会执行 → 锁永远不释放
    mtx.unlock();
}
```

只用一个 return 就能造成同样问题。所以**永远不要裸用 lock/unlock**。

## 3. RAII 锁：std::lock_guard

构造时 lock，析构时 unlock——利用 C++ RAII 机制：

```cpp
void safe_example() {
    std::lock_guard<std::mutex> lock(mtx);  // 构造时 lock
    // 临界区，尽管随便 return 或抛异常
    // ...
}  // 离开作用域 → 析构 → 自动 unlock
```

**这是 C++ 多线程中最重要的习惯之一。**

### 用 lock_guard 修复之前的计数器问题

```cpp
#include <iostream>
#include <thread>
#include <mutex>

int counter = 0;
std::mutex mtx;

void increment() {
    for (int i = 0; i < 100000; ++i) {
        std::lock_guard<std::mutex> lock(mtx);
        ++counter;
    }
}

int main() {
    std::thread t1(increment);
    std::thread t2(increment);
    t1.join();
    t2.join();
    std::cout << counter << "\n";  // 稳定 200000
    return 0;
}
```

## 4. std::unique_lock——更灵活的锁

`unique_lock` 和 `lock_guard` 一样是 RAII 锁，但多了以下能力：

### 延迟加锁 (defer_lock)
```cpp
std::unique_lock<std::mutex> lock(mtx, std::defer_lock);  // 创建但先不加锁
// ... 做一些不需要锁的事情 ...
lock.lock();  // 需要时才加锁
```

### 提前解锁
```cpp
std::unique_lock<std::mutex> lock(mtx);
// 临界区...
lock.unlock();  // 提前释放锁，不等到析构
// 其他线程可以继续前进了
// 注意：之后不能再 lock() 除非你知道自己在做什么
```

### 移动语义
```cpp
std::unique_lock<std::mutex> get_lock() {
    std::unique_lock<std::mutex> lock(mtx);
    // ...
    return lock;  // 移动，所有权转移
}
```

### unique_lock 与 lock_guard 对比

| 特性 | lock_guard | unique_lock |
|------|-----------|-------------|
| 构造时加锁 | ✅ | ✅（可 defer 取消） |
| 析构时解锁 | ✅ | ✅ |
| 手动 unlock/lock | ❌ | ✅ |
| 移动语义 | ❌ | ✅ |
| 条件变量 wait() 支持 | ❌ | ✅ |
| 性能开销 | 最小 | 稍大（维护锁状态标志） |

**使用建议**：
- 默认用 `lock_guard`
- 需要提前解锁、条件变量、转移所有权时用 `unique_lock`

## 5. 保护类内数据的标准模式

```cpp
class BankAccount {
    int balance_ = 0;
    mutable std::mutex mtx_;  // mutable 允许 const 方法加锁
public:
    void deposit(int amount) {
        std::lock_guard<std::mutex> lock(mtx_);
        balance_ += amount;
    }

    void withdraw(int amount) {
        std::lock_guard<std::mutex> lock(mtx_);
        if (amount > balance_) throw std::runtime_error("insufficient funds");
        balance_ -= amount;
    }

    int get_balance() const {
        std::lock_guard<std::mutex> lock(mtx_);  // mutable 让这里合法
        return balance_;
    }
};
```

### 设计原则
1. **封装**：mutex 和数据在同一层次（通常是同一个类），都是私有的
2. **每个公开接口都加锁**：不要假设调用者会帮你加锁
3. **不返回内部指针/引用**：
   ```cpp
   const std::string& get_name() const {
       std::lock_guard<std::mutex> lock(mtx_);
       return name_;  // ❌ 危险！调用者拿到引用后可以不通过锁访问
   }
   ```
4. **传参/返回时拷贝数据**：
   ```cpp
   std::string get_name() const {
       std::lock_guard<std::mutex> lock(mtx_);
       return name_;  // √ 返回拷贝
   }
   ```

## 6. 递归锁：std::recursive_mutex

同一个线程可以多次 lock 同一个 recursive_mutex，不会死锁：

```cpp
std::recursive_mutex rmtx;

void f() {
    std::lock_guard<std::recursive_mutex> lock(rmtx);
    // 干点事
    g();  // g() 里又去锁同一个 rmtx，不会死锁
}

void g() {
    std::lock_guard<std::recursive_mutex> lock(rmtx);  // 同一线程，没问题
}
```

但 recursive_mutex **通常意味着设计有问题**——说明你的函数层级不清晰。绝大多数场景用普通 `std::mutex` 就够了。

## 7. 锁的粒度

临界区的大小（锁的持有时间）叫**锁的粒度**：

```cpp
// ❌ 粒度太大：整个函数都锁着
void process_data(std::vector<int>& data) {
    std::lock_guard<std::mutex> lock(mtx);
    // 1. 读取文件（可能很慢）
    // 2. 复杂计算
    // 3. 写回结果
    // 其他线程全程在等...
}

// ✅ 粒度小：只保护共享数据的访问
void process_data(std::vector<int>& data) {
    std::vector<int> local_data;
    {
        std::lock_guard<std::mutex> lock(mtx);
        local_data = data;  // 只拷贝共享数据
    }  // 尽快解锁
    // 在本地处理，不影响其他线程
    process(local_data);
    {
        std::lock_guard<std::mutex> lock(mtx);
        data = std::move(local_data);
    }
}
```

原则：**锁的粒度要尽可能小，但也要足够大以保证操作的原子性。**