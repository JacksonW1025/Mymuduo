#pragma once

#include "EventLoop.h"
#include "noncopyable.h"
#include "Thread.h"
#include <functional>
#include <memory>
#include <string>
#include <mutex> // 互斥锁      
#include <condition_variable> // 条件变量   

class EventLoop;

class EventLoopThread : noncopyable{
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>; // 线程初始化回调
    EventLoopThread(const ThreadInitCallback &cb = ThreadInitCallback(), const std::string &name = std::string()); 
    ~EventLoopThread(); // 析构函数
    EventLoop* startLoop(); // 启动EventLoop
private:
    void threadFunc(); // 线程函数
    
    EventLoop* loop_; // 保存EventLoop对象的指针
    bool exiting_; // 是否退出
    Thread thread_; // 保存Thread对象的指针
    std::mutex mutex_; // 互斥锁
    std::condition_variable cond_; // 条件变量
    ThreadInitCallback callback_; // 线程初始化回调

};
    