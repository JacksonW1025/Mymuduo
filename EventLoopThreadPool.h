#pragma once

#include "EventLoop.h"
#include "EventLoopThread.h"
#include <noncopyable.h>
#include <functional>
#include <memory>
#include <string>
#include <vector>

class EventLoopThreadPool : noncopyable
{
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    EventLoopThreadPool(EventLoop *baseLoop, const std::string &nameArg);
    ~EventLoopThreadPool() = default;

    void setThreadNum(int numThreads) { numThreads_ = numThreads; }
    void start(const ThreadInitCallback &cb = ThreadInitCallback());

    // 如果工作在多线程中，baseLoop_默认以轮询的方式分配channel给subloop
    EventLoop *getNextLoop();

    std::vector<EventLoop *> getAllLoops(); 

    bool started() const { return started_; }
    const std::string &name() const { return name_; }
    
private:
    EventLoop *baseLoop_; // 主线程的EventLoop，一般的epolldemo用这种，负责处理新连接和读写事件；在这里只负责处理新连接
    std::string name_; // 线程池的名字
    bool started_; // 是否启动
    int numThreads_; // 线程数量
    int next_; // 下一个要启动的线程的索引
    std::vector<std::unique_ptr<EventLoopThread>> threads_; // 线程池
    std::vector<EventLoop*> loops_; // 线程池中的EventLoop
    
};