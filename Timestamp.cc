#include "Timestamp.h"
#include <time.h>
#include <iostream>

Timestamp::Timestamp() : microSecondsSinceEpoch_(0) {} // 默认构造函数，初始化微秒数为0

Timestamp::Timestamp(int64_t microSecondsSinceEpoch)
    : microSecondsSinceEpoch_(microSecondsSinceEpoch) {} // 带参数的构造函数，初始化微秒数为传入的值

// 不需要加static关键字，因为这个函数已经在类中声明为static了
Timestamp Timestamp::now()
{
    // time_t ti = time(nullptr); // 获取当前时间
    // return Timestamp(ti); // 将时间转换为微秒数并返回

    return Timestamp(time(nullptr)); // 将时间转换为微秒数并返回
}

std::string Timestamp::toString() const
{
    char buf[128] = {0}; // 定义一个128字节的字符数组，用于存储时间字符串
    tm* tm_time = localtime(&microSecondsSinceEpoch_); // 将微秒数转换为本地时间
    snprintf(buf, sizeof(buf), "%4d/%02d/%02d %02d:%02d:%02d", 
        tm_time->tm_year + 1900, // 年份从1900开始，所以要加1900
        tm_time->tm_mon + 1,     // 月份从0开始，所以要加1
        tm_time->tm_mday,
        tm_time->tm_hour,
        tm_time->tm_min,
        tm_time->tm_sec); // 格式化时间字符串
    return std::string(buf); 
}

int main(){
    //在Ubuntu系统上编译时，使用g++ -std=c++11 -o Timestamp Timestamp.cc
    std::cout << Timestamp::now().toString() << std::endl; // 输出当前时间
    return 0;
}