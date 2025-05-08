#pragma once
#include "noncopyable.h"
#include <string>

// 定义一个宏，用于打印日志，其中LogmsgFormat是日志格式，...是可变参数
// 这样定义宏是为了方便使用，避免每次都要写一大堆代码，这种宏定义方式在C++中很常见
// 这个宏中使用了do while循环，主要是为了避免宏定义中的语句在使用时出现语法错误
// 例如，如果在宏定义中使用了if语句，而在使用宏时没有加上大括号，那么就会出现语法错误
// \符号表示换行，表示这个宏定义可以跨多行
// 在实际使用时，只需要调用LOG_INFO宏，例如LOG_INFO("Hello %s", "world")，就可以打印日志了
// snprintf函数用于格式化输出，类似于C语言中的printf函数
// snprintf函数的第一个参数是输出缓冲区，第二个参数是缓冲区大小，第三个参数是格式化字符串，后面的参数是可变参数
// ##__VA_ARGS__这种写法是C++11中的可变参数模板，用于表示可变参数的值，和...的区别是，...表示可变参数的个数，而##__VA_ARGS__表示可变参数的值
// LOG_INFO("%s %d",arg1, arg2)
#define LOG_INFO(LogmsgFormat,...) \
    do { \
        Logger &logger = Logger::instance(); \
        logger.setLogLevel(LogLevel::INFO); \
        char buf[1024] = {0}; \
        snprintf(buf, sizeof(buf), LogmsgFormat, ##__VA_ARGS__); \
        logger.log(buf); \
    } while (0)

#define LOG_ERROR(LogmsgFormat,...) \
    do { \
        Logger &logger = Logger::instance(); \
        logger.setLogLevel(LogLevel::ERROR); \
        char buf[1024] = {0}; \
        snprintf(buf, sizeof(buf), LogmsgFormat, ##__VA_ARGS__); \
        logger.log(buf); \
    } while (0)

#define LOG_FATAL(LogmsgFormat,...) \
    do { \
        Logger &logger = Logger::instance(); \
        logger.setLogLevel(LogLevel::FATAL); \
        char buf[1024] = {0}; \
        snprintf(buf, sizeof(buf), LogmsgFormat, ##__VA_ARGS__); \
        logger.log(buf); \
        exit(-1); \
    } while (0)
    
// 如果定义了MUDEBUG宏，则编译器会编译下面的代码,这是因为需要避免在release版本中输出调试信息，造成性能下降
#ifdef MUDEBUG 

#define LOG_DEBUG(LogmsgFormat,...) \
    do { \
        Logger &logger = Logger::instance(); \
        logger.setLogLevel(LogLevel::DEBUG); \
        char buf[1024] = {0}; \
        snprintf(buf, sizeof(buf), LogmsgFormat, ##__VA_ARGS__); \
        logger.log(buf); \
    } while (0)
#else
    #define LOG_DEBUG(LogmsgFormat,...) // 如果没有定义MUDEBUG宏，则LOG_DEBUG宏为空，不会输出任何日志
#endif

// 定义日志的级别 INFO ERROR FATAL DEBUG
// 在 C++11 及以后，使用 enum class 而不是传统的 enum 是推荐的做法，虽然需要多写一些代码（加上 LogLevel::），但提供了更好的类型安全性和命名空间管理。
enum class LogLevel
{
    INFO,       // INFO级别日志，普通信息
    ERROR,      // ERROR级别日志，错误信息
    FATAL,      // FATAL级别日志，core dump
    DEBUG       // DEBUG级别日志，调试信息
};

class Logger : noncopyable
{
public:
    // 获取日志唯一的实例对象，static是为了让这个函数可以在类外部调用，而不需要创建Logger对象
    // 这个函数是一个静态成员函数，返回Logger类的唯一实例
    static Logger& instance();
    // 设置日志级别
    void setLogLevel(LogLevel level);
    // 写日志
    void log(std::string msg);
private:
    LogLevel logLevel_; // 日志级别，_表示私有成员变量，加在后面是为了避免与成员函数名冲突，区分成员变量和成员函数，避免和系统变量冲突
    Logger(){}
};
