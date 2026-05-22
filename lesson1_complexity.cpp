#include <iostream>
#include <chrono>
#include <vector>
#include <cmath>
#include <algorithm>
#include <numeric>

// O(1) — 直接返回第一个元素
int first_element(const std::vector<int>& v) {
    return v.empty() ? -1 : v[0];
}

// O(n) — 遍历一遍求和
long long sum_all(const std::vector<int>& v) {
    long long total = 0;
    for (int x : v) total += x;
    return total;
}

// O(n^2) — 打印所有数对
long long count_pairs(const std::vector<int>& v) {
    long long count = 0;
    for (size_t i = 0; i < v.size(); i++)
        for (size_t j = i + 1; j < v.size(); j++)
            count++;
    return count;
}

// O(log n) — 二分查找
int binary_search(const std::vector<int>& v, int target) {
    int lo = 0, hi = v.size() - 1;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        if (v[mid] == target) return mid;
        else if (v[mid] < target) lo = mid + 1;
        else hi = mid - 1;
    }
    return -1;
}

// 计时工具
template<typename Func>
double measure(Func f, int n) {
    std::vector<int> v(n);
    std::iota(v.begin(), v.end(), 0);  // 0, 1, 2, ...

    auto start = std::chrono::high_resolution_clock::now();
    volatile auto result = f(v);  // volatile 防止编译器优化掉
    auto end = std::chrono::high_resolution_clock::now();

    return std::chrono::duration<double, std::micro>(end - start).count();
}

int main() {
    std::cout << "========== 复杂度直观对比 ==========\n\n";

    for (int n : {100, 1000, 10000, 100000}) {
        std::cout << "n = " << n << ":\n";
        std::cout << "  O(1)     " << measure([](const auto& v) { return first_element(v); }, n) << " us\n";
        std::cout << "  O(log n) " << measure([](const auto& v) { return binary_search(v, -1); }, n) << " us\n";
        std::cout << "  O(n)     " << measure([](const auto& v) { return sum_all(v); }, n) << " us\n";
        if (n <= 10000) {
            std::cout << "  O(n^2)   " << measure([](const auto& v) { return count_pairs(v); }, n) << " us\n";
        } else {
            std::cout << "  O(n^2)   (跳过，太慢了)\n";
        }
        std::cout << "\n";
    }

    // 空间复杂度演示
    std::cout << "========== 空间复杂度示例 ==========\n";
    std::cout << "O(1) sum_all:        只用了 total 变量\n";
    std::cout << "O(n) 复制数组:       分配了 n 大小新数组\n";
    std::cout << "O(n) 斐波那契递归:   调用栈深度 = n\n";

    return 0;
}
