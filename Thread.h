#pragma once

#include "noncopyable.h"
#include <functional>
#include <thread> // C++11 线程库
#include <memory>
#include <string>
#include <atomic>


class Thread : noncopyable
{
public:
    using ThreadFunc = std::function<void()>;
    explicit Thread(ThreadFunc, const std::string& name = std::string()); // ThreadFunc是一个函数类型，表示线程函数
    ~Thread();

    void start();
    void join();

    bool started() const { return started_; }
    pid_t tid() const { return tid_; }
    const std::string& name() const { return name_; }

    static int numCreated() { return numCreated_; }

private:
    void setDefaultName();
    
    bool started_;
    bool joined_;
    std::shared_ptr<std::thread> thread_; // 不能直接使用std::thread，因为std::thread会直接启动
    pid_t tid_;
    ThreadFunc func_; // 线程函数
    std::string name_;
    static std::atomic_int numCreated_;

};