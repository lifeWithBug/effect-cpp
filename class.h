#pragma once

#include <iostream>
using namespace std;

// Widget 类用于演示 C++ 中的构造函数、拷贝构造函数和赋值运算符重载。
class Widget
{
public:
    // 默认构造函数
    Widget();

    explicit Widget(const char *str);

    // 移动构造函数：用于从一个右值 Widget 对象创建新对象
    // 使用 noexcept 关键字表示程序员承诺该函数不会抛出异常，
    // 抛出异常会导致程序终止，调用std::terminate()函数。
    Widget(Widget &&rhs) noexcept;

    // 移动赋值运算符重载：用于将一个右值 Widget 对象的状态移动到另一个已存在对象
    Widget &operator=(Widget &&rhs) noexcept;

    // 拷贝构造函数：用于从另一个 Widget 对象创建新对象
    Widget(const Widget &rhs);
    // 赋值运算符重载：用于将一个 Widget 对象的状态复制到另一个已存在对象
    Widget &operator=(const Widget &rhs);

    void swap(Widget &rhs) noexcept;

    void print() const;

    ~Widget();

private:
    char *data;
    int size;
};