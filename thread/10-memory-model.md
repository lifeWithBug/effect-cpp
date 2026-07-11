# C++ 多线程笔记 10：C++ 内存模型

## 1. 问题来源：指令重排

### 编译器重排
编译器为了优化，可能改变指令顺序——只要不改变单线程的 observable behavior：

```cpp
int a = 0;
bool flag = false;

// 线程 1
a = 42;
flag = true;   // 编译器可能把这一行移到 a = 42 之前！
```

### CPU 重排
现代 CPU 采用**乱序执行**。CPU 从自己的角度看是顺序的，但从其他 CPU 看来可能是乱序的：

```
CPU 1 写 a = 42
CPU 1 写 flag = true
CPU 2 读 flag = true
CPU 2 读 a     → 可能读到 0（a 的写还没刷新到主存）
```

这就是**内存序问题**。你需要告诉编译器和 CPU，哪些地方的重排是不允许的。

## 2. C++ 的六大内存序

C++11 为原子操作定义了六种内存序：

```cpp
std::atomic<int> a{0};

// 默认：顺序一致性
a.store(42);                        // memory_order_seq_cst
int x = a.load();                   // memory_order_seq_cst

// 显式指定内存序
a.store(42, std::memory_order_release);
int x = a.load(std::memory_order_acquire);
a.fetch_add(1, std::memory_order_relaxed);
```

### 按顺序保证强度从高到低

| 内存序 | 保证 | 性能代价 | 使用场景 |
|--------|------|---------|---------|
| `seq_cst` | 所有线程看到全局一致的操作顺序 | 最大 | 默认，99% 场景 |
| `acq_rel` | acquire + release，用于 read-modify-write | 中等 | CAS loop 等 RMW 操作 |
| `acquire` | 之后的读写不重排到 acquire 之前 | 较低 | 读同步标志 |
| `release` | 之前的读写不重排到 release 之后 | 较低 | 写同步标志 |
| `consume` | 只保证数据依赖关系（编译器通常降级为 acquire） | — | 几乎不用 |
| `relaxed` | 只保证原子性，不保证顺序 | 最小 | 计数器（不用于同步） |

## 3. 顺序一致性 (seq_cst)——默认、安全、最昂贵

```cpp
std::atomic<int> x{0}, y{0};
int r1, r2;

// 线程 1
x.store(1);           // A
r1 = y.load();        // B

// 线程 2
y.store(1);           // C
r2 = x.load();        // D
```

顺序一致性保证：所有线程看到的操作顺序像在全局时间线上按某种顺序排列。可以有 `(r1=1, r2=1)`、`(r1=0, r2=1)` 等结果，但不会是 `(r1=0, r2=0)`（因为如果 A 和 C 都执行了，至少有一个的 store 被另一个的 load 看到）。

**理解方式**：把整个程序的原子操作想象成按某种顺序依次执行，所有线程看到的顺序都一样。

## 4. Release-Acquire 语义——最实用的通信模式

这是 C++ 中最常用的内存序模式，实现**生产者-消费者同步**：

```cpp
std::atomic<bool> ready{false};
int data = 0;

// ---- 线程 1（生产者）----
data = 42;                                     // A
ready.store(true, std::memory_order_release);   // B

// ---- 线程 2（消费者）----
while (!ready.load(std::memory_order_acquire));  // C
// 保证：现在 data == 42
```

### 保证了什么？

- **Release**：所有在 release store **之前**的写操作，对 acquire 到同一变量的线程可见
- **Acquire**：acquire load **之后**的读操作，能看到 release store 之前的写
- 线程 2 看到 `ready == true` 时，也一定能看到 `data == 42`

### 为什么叫 release 和 acquire？

```
Thread 1:              Thread 2:
[写 data = 42]         [忙等直到 ready == true ← acquire]
[release:  写 ready]   [读 data → 保证看到 42]
```

- Thread 1 **释放**了 data 的所有权，Thread 2 可以安全读取
- Thread 2 **获取**了 data 的所有权，保证能看到之前的修改

## 5. Relaxed 语义——仅保证原子性

`memory_order_relaxed` 只保证操作本身是原子的，不提供任何顺序保证：

```cpp
std::atomic<int> counter{0};

// 只做计数，不用于同步其他数据——perfect for relaxed
void worker() {
    for (int i = 0; i < 1000; ++i) {
        counter.fetch_add(1, std::memory_order_relaxed);
    }
}

int main() {
    std::thread t1(worker), t2(worker);
    t1.join(); t2.join();
    std::cout << counter << "\n";  // 稳定 2000
}
```

**为什么需要 relaxed？** 性能。在 x86 上 relaxed 和 seq_cst 差别不大（x86 的强内存模型本来就很严格），但在 ARM/PowerPC 等弱内存模型的 CPU 上，seq_cst 会插入额外的 fence 指令，开销大很多。

## 6. 获取-释放传递性

```cpp
std::atomic<int> a{0}, b{0};
int data = 0;

// 线程 1
data = 1;                     // A
a.store(1, std::release);     // B

// 线程 2
while (a.load(std::acquire) != 1);  // C
b.store(1, std::release);           // D

// 线程 3
while (b.load(std::acquire) != 1);  // E
// 保证：data == 1  ✓
```

因为 B release 同步 C acquire，D release 同步 E acquire，所以 A 的副作用对 E 之后可见。

## 7. std::atomic_thread_fence

除了在原子操作上指定内存序，还可以用独立的 fence：

```cpp
int data = 0;
std::atomic<bool> ready{false};

// 线程 1
data = 42;
std::atomic_thread_fence(std::memory_order_release);
ready.store(true, std::memory_order_relaxed);  // relaxed 也够了

// 线程 2
while (!ready.load(std::memory_order_relaxed));
std::atomic_thread_fence(std::memory_order_acquire);
// 保证：data == 42
```

fence 更灵活也更难推理，大多数人用原子操作上的内存序就够了。

## 8. 实践建议

### 原则 1：默认用 seq_cst

```cpp
std::atomic<int> a{0};
a.store(42);       // 等价于 seq_cst，最容易推理
int x = a.load();
```

编译器在 x86 上会把 seq_cst 优化成普通指令（因为 x86 的强内存模型不需要额外的 fence），所以**不需要担心性能问题**，除非你在 ARM 等弱内存模型平台上做真的性能敏感优化。

### 原则 2：只有性能热点才考虑降级

```cpp
// 降级为 relaxed，因为 counter 只用于打点统计
stats_counter.fetch_add(1, std::memory_order_relaxed);
```

### 原则 3：用 release-acquire 做同步

```cpp
// 标准的生产者-消费者模式
producer_data = result;
ready.store(true, std::memory_order_release);

// 消费者端
while (!ready.load(std::memory_order_acquire));
use(producer_data);
```

### 原则 4：注释你的内存序

```cpp
// 非默认内存序必须有注释说明原因
value.store(42, std::memory_order_release);
// ^ 保证之前的数据写入对 acquire 端可见
```

### 核心思想

```
顺序一致性  (seq_cst)     → 最容易推理，默认选择
释放-获取  (release-acquire) → 生产者-消费者通信
弱         (relaxed)     → 只做计数，不做同步
```