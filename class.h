#pragma once

#include <iostream>
using namespace std;

// Widget 类用于演示 C++ 中的构造函数、拷贝构造函数和赋值运算符重载。
class Widget
{
public:
    // 默认构造函数
    Widget();

    // 拷贝构造函数：用于从另一个 Widget 对象创建新对象
    Widget(const Widget &rhs);

    // 赋值运算符重载：用于将一个 Widget 对象的状态复制到另一个已存在对象
    Widget &operator=(const Widget &rhs);
};