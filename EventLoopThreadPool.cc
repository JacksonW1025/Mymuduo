#include "EventLoopThreadPool.h"
#include "EventLoopThread.h"


EventLoopThreadPool::EventLoopThreadPool(EventLoop *baseLoop, const std::string &nameArg)
    : baseLoop_(baseLoop),
      name_(nameArg),
      started_(false),
      numThreads_(0),
      next_(0)
{
}

EventLoopThreadPool::~EventLoopThreadPool() = default;

void EventLoopThreadPool::start(const ThreadInitCallback &cb)
{
    started_ = true;
    // 创建numThreads_个线程，每个线程创建一个EventLoop，并启动
    for (int i = 0; i < numThreads_; ++i)
    {
        char buf[name_.size() + 32];
        snprintf(buf, sizeof buf, "%s%d", name_.c_str(), i);
        EventLoopThread *t = new EventLoopThread(cb, buf);
        threads_.push_back(std::unique_ptr<EventLoopThread>(t));
        loops_.push_back(t->startLoop());//底层创建线程，绑定一个新的EventLoop，并返回该loop的指针
    }

    // 整个服务端只有一个线程，负责main loop，处理accept事件
    if (numThreads_ == 0 && cb)
    {
        cb(baseLoop_);
    }
}

EventLoop *EventLoopThreadPool::getNextLoop()
{
    EventLoop *loop = baseLoop_;
    if (!loops_.empty())
    {
        loop = loops_[next_];
    }
}

std::vector<EventLoop *> EventLoopThreadPool::getAllLoops()
{
    EventLoop *loop = baseLoop_;
    
    if (!loops_.empty()) // 通过轮询获取下一个处理事件的loop
    {
        loop = loops_[next_];
        ++next_;
        if (next_ >= loops_.size())
        {
            next_ = 0;
        }
    }
    return std::vector<EventLoop *>(1, loop);
}