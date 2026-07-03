# C 语言动态内存分配三大核心函数总结

## 1. 函数概览

| 函数 | 头文件 | 是否初始化 | 是否可调整大小 | 释放方式 |
| :--- | :--- | :--- | :--- | :--- |
| **`malloc`** | `<stdlib.h>` | ❌ 不初始化（含垃圾值） | ❌ 否 | `free()` |
| **`calloc`** | `<stdlib.h>` | ✅ 全部初始化为 0 | ❌ 否 | `free()` |
| **`realloc`** | `<stdlib.h>` | 保留原数据 | ✅ 是 | `free()` |

---

## 2. 详细说明

### 2.1 `malloc` —— 原始内存分配

```c
void* malloc(size_t size);
```

- **功能**：从堆中分配 `size` 个字节的连续内存。
- **初始化**：不会初始化内存，内容为随机值（脏数据）。
- **返回值**：成功返回指向内存块的指针，失败返回 `NULL`。

**典型用法**：

```c
int* p = (int*)malloc(10 * sizeof(int));
if (p == NULL) {
    // 处理分配失败
}
```

**适用场景**：申请后立即会覆写所有数据的场景（如文件读入缓冲区）。

---

### 2.2 `calloc` —— 分配并清零

```c
void* calloc(size_t num, size_t size);
```

- **功能**：分配 `num` 个大小为 `size` 字节的元素，总大小为 `num * size` 字节。
- **初始化**：全部初始化为 0（比 `malloc` + `memset` 可能更高效）。
- **返回值**：成功返回指向内存块的指针，失败返回 `NULL`。

**典型用法**：

```c
int* p = (int*)calloc(10, sizeof(int));
if (p == NULL) {
    // 处理分配失败
}
// 此时所有元素已为 0
```

**适用场景**：需要初始值为 0 的结构体数组、字符串缓冲区等。

> ⚠️ **注意**：`num * size` 可能发生整数溢出（尤其数值较大时）。

---

### 2.3 `realloc` —— 调整内存大小

```c
void* realloc(void* ptr, size_t new_size);
```

- **功能**：调整已分配内存块 `ptr` 的大小为 `new_size` 字节。
- **初始化**：保留原内存中的数据（新增加的部分不初始化）。

**行为细节**：

- `new_size > 旧大小`：可能原地扩展，或迁移到新地址并自动拷贝旧数据。
- `new_size < 旧大小`：截断，释放多余部分。
- `ptr == NULL`：等同于 `malloc(new_size)`。
- `new_size == 0`：等同于 `free(ptr)`，可能返回 `NULL`（不推荐）。

- **返回值**：成功返回新内存块指针（可能与 `ptr` 相同或不同），失败返回 `NULL`。

> ⚠️ **关键陷阱 —— 必须用临时指针接收返回值**：

```c
// ❌ 错误写法（失败时内存泄漏）
ptr = realloc(ptr, new_size);

// ✅ 正确写法
void* new_ptr = realloc(ptr, new_size);
if (new_ptr == NULL) {
    // 原 ptr 依然有效，可继续使用或释放
} else {
    ptr = new_ptr;
}
```

**适用场景**：动态数组扩容/缩容（如 `vector` 的底层实现）。

---

## 3. 共同注意事项

1. **必须检查返回值**是否为 `NULL`，内存耗尽时所有函数都会返回空指针。
2. **必须配对使用 `free()`**，否则造成内存泄漏。
3. **`free()` 后应将指针置为 `NULL`**，防止野指针。
4. **禁止重复释放**同一块内存（未定义行为）。
5. **只能释放由上述函数分配的内存**，不能释放栈上或静态内存。

---

## 4. 示例代码（综合对比）

```c
#include <stdio.h>
#include <stdlib.h>

int main() {
    // 1. malloc - 未初始化
    int* p1 = (int*)malloc(3 * sizeof(int));
    if (p1) {
        p1[0] = 10;  // 必须手动赋值
        p1[1] = 20;
        p1[2] = 30;
        free(p1);
        p1 = NULL;
    }

    // 2. calloc - 自动清零
    int* p2 = (int*)calloc(3, sizeof(int));
    if (p2) {
        // p2[0], p2[1], p2[2] 均为 0
        p2[0] = 100;
        free(p2);
        p2 = NULL;
    }

    // 3. realloc - 扩容
    int* p3 = (int*)malloc(2 * sizeof(int));
    if (p3) {
        p3[0] = 1;
        p3[1] = 2;

        int* new_p3 = (int*)realloc(p3, 4 * sizeof(int));
        if (new_p3) {
            p3 = new_p3;
            p3[2] = 3;  // 新位置赋值
            p3[3] = 4;
        }
        free(p3);
        p3 = NULL;
    }

    return 0;
}
```

---

## 5. 选择建议

| 需求 | 推荐函数 |
| :--- | :--- |
| 只需原始内存，随后立即全部覆写 | `malloc` |
| 需要初始值全为 0 的数组/结构体 | `calloc` |
| 已有内存块需要扩大或缩小 | `realloc` |
| 追求极简代码，数量不大且需清零 | `calloc` |

---

## 6. 补充：内存连续性说明

`malloc`、`calloc`、`realloc` 返回的内存块在**虚拟地址**上都是连续的，可以当作数组使用。

- **虚拟地址**：连续（对程序员透明）。
- **物理地址**：由操作系统通过 MMU 映射，可能不连续（程序员无需关心）。

`calloc` 与 `malloc` 在连续性上没有区别，唯一区别是 `calloc` 会清零。
