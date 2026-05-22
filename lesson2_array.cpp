#include <iostream>
#include <stdexcept>
#include <algorithm>

template <typename T>
class DynamicArray {
    T* data_;
    size_t size_;
    size_t capacity_;

    void resize(size_t new_cap) {
        T* new_data = new T[new_cap];
        for (size_t i = 0; i < size_; i++)
            new_data[i] = std::move(data_[i]);
        delete[] data_;
        data_ = new_data;
        capacity_ = new_cap;
    }

public:
    DynamicArray(size_t cap = 4)
        : data_(new T[cap]), size_(0), capacity_(cap) {}

    ~DynamicArray() { delete[] data_; }

    // O(1) — 按下标访问
    T& operator[](size_t idx) {
        if (idx >= size_) throw std::out_of_range("index out of range");
        return data_[idx];
    }

    // O(1) 均摊 — 末尾插入
    void push_back(const T& val) {
        if (size_ == capacity_)
            resize(capacity_ * 2);  // 扩容策略：翻倍
        data_[size_++] = val;
    }

    // O(1) — 末尾删除
    void pop_back() {
        if (size_ == 0) throw std::out_of_range("empty array");
        --size_;
    }

    // O(n) — 中间插入
    void insert(size_t idx, const T& val) {
        if (idx > size_) throw std::out_of_range("index out of range");
        if (size_ == capacity_)
            resize(capacity_ * 2);
        for (size_t i = size_; i > idx; i--)
            data_[i] = data_[i - 1];  // 后移
        data_[idx] = val;
        ++size_;
    }

    // O(n) — 中间删除
    void erase(size_t idx) {
        if (idx >= size_) throw std::out_of_range("index out of range");
        for (size_t i = idx; i < size_ - 1; i++)
            data_[i] = data_[i + 1];  // 前移
        --size_;
    }

    size_t size() const { return size_; }
    size_t capacity() const { return capacity_; }

    void print() const {
        std::cout << "[";
        for (size_t i = 0; i < size_; i++) {
            std::cout << data_[i];
            if (i + 1 < size_) std::cout << ", ";
        }
        std::cout << "] (size=" << size_ << ", cap=" << capacity_ << ")\n";
    }
};

int main() {
    DynamicArray<int> arr;

    std::cout << "=== 末尾插入 (O(1) 均摊) ===\n";
    for (int i = 10; i <= 50; i += 10) arr.push_back(i);
    arr.print();

    std::cout << "\n=== 随机访问 (O(1)) ===\n";
    std::cout << "arr[2] = " << arr[2] << "\n";

    std::cout << "\n=== 中间插入 (O(n)) ===\n";
    arr.insert(2, 99);
    arr.print();

    std::cout << "\n=== 中间删除 (O(n)) ===\n";
    arr.erase(1);
    arr.print();

    std::cout << "\n=== 末尾删除 (O(1)) ===\n";
    arr.pop_back();
    arr.print();
}
