#pragma once

/*
noncopyable被设计为一个基类，禁止派生类的拷贝构造和赋值操作。
这意味着任何继承自noncopyable的类都不能被复制或赋值。
这通常用于防止对象在不希望的情况下被复制，确保类的实例是唯一的。

例如，单例模式中的类通常会继承自noncopyable，以确保只有一个实例存在。
在C++中，使用delete关键字可以显式地禁用拷贝构造函数和赋值操作符。
在这个例子中，noncopyable类的拷贝构造函数和赋值操作符被删除了。
这意味着任何尝试复制或赋值noncopyable类的实例都会导致编译错误。

派生类对象可以正常地构造和析构。
*/

class noncopyable {
public:
    noncopyable(const noncopyable&) = delete; // Disable copy constructor
    noncopyable& operator=(const noncopyable&) = delete; // Disable copy assignment
protected:
    noncopyable() = default; // Default constructor
    ~noncopyable() = default; // Default destructor
    // Allow derived classes to be destructed
    
};