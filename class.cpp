#include "class.h"
#include <iostream>
#include <utility> // for std::exchange
#include <cstring>

// 默认构造
Widget::Widget() : data(nullptr), size(0) {}

// 带参构造函数
Widget::Widget(const char *str)
    : data(nullptr), size(0)
{
    if (str)
    {
        size = strlen(str);
        data = new char[size + 1];
        strcpy(data, str);
    }
}

// 移动构造函数
Widget::Widget(Widget &&rhs) noexcept
    : data(std::exchange(rhs.data, nullptr)), size(std::exchange(rhs.size, 0)) {}

// 移动赋值运算符重载
Widget &Widget::operator=(Widget &&rhs) noexcept
{
    if (this != &rhs)
    {
        delete[] data;
        data = std::exchange(rhs.data, nullptr);
        size = std::exchange(rhs.size, 0);
    }
    return *this;
}

// 拷贝构造函数
Widget::Widget(const Widget &rhs) : data(nullptr), size(rhs.size)
{
    if (rhs.data)
    {
        data = new char[size + 1];
        strcpy(data, rhs.data);
    }
}

// 拷贝赋值运算符重载
Widget &Widget::operator=(const Widget &rhs)
{
    if (this != &rhs)
    {
        /*  no swap version
        delete[] data;
        size=rhs.size;
        if(rhs.data)
        {
            data=new char[size+1];
            strcpy(data,rhs.data);
        }
        */
        Widget temp(rhs); // 利用拷贝构造函数创建临时对象
        swap(temp);       // 交换当前对象与临时对象的资源
    }
    return *this;
}

// swap
void Widget::swap(Widget &rhs) noexcept
{
    std::swap(data, rhs.data);
    std::swap(size, rhs.size);
}

// 析构函数
Widget::~Widget()
{
    delete[] data;
}

// 辅助方法
void print(const Widget &w)
{
    if (w.data)
    {
        std::cout << w.data << " (size: " << w.size << ")" << std::endl;
    }
    else
    {
        std::cout << "nullptr" << std::endl;
    }
}