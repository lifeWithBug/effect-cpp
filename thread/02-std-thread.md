# C++ 多线程笔记 02：std::thread

## 1. 创建线程的五种方式

### 1. 函数指针
```cpp
#include <iostream>
#include <thread>

void hello() {
    std::cout << "Hello from thread, thread id = "
              << std::this_thread::get_id() << "\n";
}

int main() {
    std::thread t(hello);   // 创建线程，立即执行 hello()
    t.join();                // 等待线程结束
    return 0;
}
```

### 2. 函数对象（仿函数）
```cpp
class Greeter {
    std::string msg_;
public:
    explicit Greeter(std::string msg) : msg_(std::move(msg)) {}
    void operator()() const {
        std::cout << msg_ << "\n";
    }
};

int main() {
    Greeter g("Hello from functor");
    std::thread t(g);
    t.join();
    return 0;
}
```

### 3. Lambda
```cpp
std::thread t([](int a, int b) {
    std::cout << "Sum = " << (a + b) << "\n";
}, 3, 4);
t.join();
```

### 4. 非静态成员函数

`std::thread` 启动非静态成员函数有**三种**写法，分别适用于不同场景：

### 方式 1：传递对象指针（标准写法）

```cpp
class Counter {
    int count_ = 0;
public:
    void add(int n) { count_ += n; }
    int get() const { return count_; }
};

int main() {
    Counter c;
    // 语法：&ClassName::Method, 对象指针, 参数...
    std::thread t(&Counter::add, &c, 5);
    t.join();
    std::cout << c.get() << "\n";  // 5
    return 0;
}
```

`&Counter::add` 是一个**成员函数指针**（类型 `void (Counter::*)(int)`）。`&c` 作为第二个参数，被 std::thread 内部当作调用对象。

**注意**：这里传的是**裸指针**。如果线程运行时对象已经销毁，行为未定义。

### 方式 2：用 Lambda 捕获（推荐，最安全灵活）

```cpp
Counter c;
std::thread t([&c] {
    c.add(5);           // 引用捕获
});
t.join();
```

用 lambda 的优势：
- 可以**同时调用多个函数**，不限于单个成员函数
- 可以**混合**调用多个对象的函数
- 用 `shared_ptr` 保证对象生命周期安全：

```cpp
auto c = std::make_shared<Counter>();
std::thread t([c] {        // 拷贝 shared_ptr，引用计数 +1
    c->add(5);
    c->add(10);
    std::cout << c->get() << "\n";
});
t.detach();  // 安全：即使 main 结束，shared_ptr 在线程里还活着
```

### 方式 3：用 std::bind

```cpp
Counter c;
std::thread t(std::bind(&Counter::add, &c, 5));
// 等价于 thread(&Counter::add, &c, 5)
t.join();
```

日常推荐**方式 1**（简洁）或**方式 2**（灵活可读），方式 3 和方式 1 本质相同。

### 生命周期问题是关键

无论哪种方式，都要保证对象在线程执行期间存活：

```cpp
// ❌ 错误：c 在线程执行前就销毁了
std::thread t;
{
    Counter c;
    t = std::thread(&Counter::add, &c, 5);
}
// c 已销毁，但线程可能还没开始执行 add
t.join();

// ✅ 正确：用 shared_ptr 管理
auto c = std::make_shared<Counter>();
std::thread t([c] { c->add(5); });
t.join();
```

### 5. 静态成员函数

```cpp
class Counter {
    int count_ = 0;
public:
    static void add(int n) { count_ += n; }
    int get() const { return count_; }
};

int main() {
    // 语法：&ClassName::StaticMethod, 参数...
    std::thread t(&Counter::add, 5);
    t.join();
    std::cout << c.get() << "\n";  // 5
    return 0;
}
```


## 2. 传参细节

### 默认按值拷贝
```cpp
void worker(int id, std::string text) {
    std::cout << "Thread " << id << ": " << text << "\n";
}

int main() {
    std::thread t(worker, 42, "hello");
    t.join();
    return 0;
}
```
参数会被**拷贝**到线程内部的存储中，再转发给函数。

### 传引用：必须用 std::ref
```cpp
void accumulate(int& sum, int n) {
    sum += n;
}

int main() {
    int result = 0;
    // 错误：编译错误或值拷贝
    // std::thread t(accumulate, result, 10);
    
    // 正确：显式用 ref 包装
    std::thread t(accumulate, std::ref(result), 10);
    t.join();
    std::cout << result << "\n";  // 10
    return 0;
}
```

### 传 unique_ptr（移动语义）
```cpp
void take_ownership(std::unique_ptr<int> p) {
    std::cout << *p << "\n";
}

int main() {
    auto p = std::make_unique<int>(42);
    std::thread t(take_ownership, std::move(p));  // move 进线程
    t.join();
    // p 此时为空
    return 0;
}
```

