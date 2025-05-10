#include "EventLoopThread.h"
#include "EventLoop.h"

EventLoopThread::EventLoopThread(const ThreadInitCallback &cb, const std::string &name)
    : loop_(nullptr),
      exiting_(false),
      thread_(std::bind(&EventLoopThread::threadFunc, this), name), // 绑定线程函数，bind是绑定函数，bind(&EventLoopThread::threadFunc, this)表示绑定EventLoopThread的threadFunc函数，this表示EventLoopThread对象
      mutex_(),
      cond_(), // 条件变量
      callback_(cb)
{

}


EventLoopThread::~EventLoopThread()
{
    exiting_ = true;
    if(loop_ != nullptr){
        loop_->quit();
        thread_.join();
    }
}


EventLoop* EventLoopThread::startLoop(){
    thread_.start(); // 启动底层的线程

    EventLoop* loop = nullptr;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while(loop_ == nullptr){
            cond_.wait(lock); // 等待条件变量通知
        }
        loop = loop_;
    }

    return loop;

}

// 这个方法是在单独的新线程中执行的
void EventLoopThread::threadFunc()
{
    EventLoop loop; // 创建一个独立的EventLoop对象，和线程是一一对应的

    if(callback_){
        callback_(&loop);
    }
    
    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_ = &loop;
        cond_.notify_one(); // 通知等待的线程，EventLoop对象已经创建好了
    }
    
    loop.loop(); // 执行EventLoop的loop方法，开启Poller的监听
    std::unique_lock<std::mutex> lock(mutex_);
    loop_ = nullptr;
}

