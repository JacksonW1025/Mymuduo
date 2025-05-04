#include "Logger.h"
#include <iostream>
#include "Timestamp.h"

// 获取日志唯一的实例对象，static是为了让这个函数可以在类外部调用，而不需要创建Logger对象
// 这个函数是一个静态成员函数，返回Logger类的唯一实例
// 在cpp文件中不需要加上static关键字，因为这个函数已经在类中声明为static了
Logger& Logger::instance(){
    static Logger logger; // 静态局部变量，只有在第一次调用时创建，之后调用时直接返回
    return logger; // 返回实例的地址
}

// 设置日志级别
void Logger::setLogLevel(int level){
    logLevel_ = level; // 设置日志级别
    // 这里可以添加一些其他的初始化代码，比如打开文件，设置格式等
}

// 写日志 [级别信息] time : message
void Logger::log(std::string msg){
    switch (logLevel_) // 根据日志级别输出不同的日志
    {
        case INFO:
            std::cout << "[INFO]";
            break;
        case ERROR:
            std::cout << "[ERROR]";
            break;
        case FATAL:
            std::cout << "[FATAL]";
            break;
        case DEBUG:
            std::cout << "[DEBUG]";
            break;
        default:
            break;
    }

    // 打印时间和msg
    std::cout << Timestamp::now().toString() << " : " << msg << std::endl;
}