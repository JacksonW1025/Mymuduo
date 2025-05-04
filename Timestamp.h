#pragma once
#include <iostream>
#include <string>

class Timestamp
{
public:
    Timestamp();
    explicit Timestamp(int64_t microSecondsSinceEpoch);//explicit修饰符表示这个构造函数只能被显式调用，不能被隐式转换
    static Timestamp now();       // 获取当前时间戳
    std::string toString() const; // 转换为字符串,const修饰符表示这个函数不会修改类的成员变量
private:
    int64_t microSecondsSinceEpoch_; // 微秒数
};