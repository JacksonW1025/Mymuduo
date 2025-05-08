#include "EventLoop.h"
#include "Logger.h"
#include "Poller.h"
#include "CurrentThread.h"
#include "Channel.h"

#include <sys/eventfd.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>


// __thread相当于thread_local，每个线程都有自己的t_loopInThisThread变量，互不干扰
__thread EventLoop* t_loopInThisThread = nullptr; // 定义一个__thread变量，用于缓存当前线程的EventLoop对象，为了防止一个线程创建多个EventLoop对象

// 定义默认的Poller IO复用接口的超时时间
const int kPollTimeMs = 10000;

// 创建wakeupfd，用于唤醒subReactor处理新来的channel
int createEventfd()
{
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if(evtfd < 0)
    {
        LOG_FATAL("eventfd error: %d", errno);
    }
    return evtfd;
}

EventLoop::EventLoop()
    : looping_(false)
    , quit_(false)
    , callingPendingFunctors_(false)
    , threadId_(CurrentThread::tid())
    , poller_(Poller::newDefaultPoller(this))
    , wakeupFd_(createEventfd())
    , wakeupChannel_(new Channel(this, wakeupFd_))
    , currentActiveChannel_(nullptr)
{
    LOG_DEBUG("EventLoop created %p in thread %d \n", this, threadId_);
    if(t_loopInThisThread)
    {
        LOG_FATAL("Another EventLoop %p exists in this thread %d \n", t_loopInThisThread, threadId_);
    }
    else
    {
        t_loopInThisThread = this;
    }

    // 设置wakeupfd的事件类型以及发生事件后的回调操作,bind函数用于将EventLoop::handleRead函数绑定到wakeupChannel_的读事件上
    wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
    // 每一个eventloop都将监听wakeupchannel的EPOLLIN读事件
    wakeupChannel_->enableReading();
}


EventLoop::~EventLoop()
{
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();
    ::close(wakeupFd_);
    t_loopInThisThread = nullptr;
    // 智能指针会自动释放wakeupChannel_的相关资源
}

void EventLoop::loop()
{
    looping_ = true;
    quit_ = false;

    LOG_INFO("EventLoop %p start looping \n", this);

    while(!quit_)
    {
        activeChannels_.clear();
        // 监听两类fd：1.client的fd 2.wakeupfd
        pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);
        for(Channel* channel : activeChannels_)
        {
            // Poller监听哪些channel发生事件了，然后上报给EventLoop，EventLoop再处理相应的事件
            channel->handleEvent(pollReturnTime_);
        }
        // 执行当前EventLoop事件循环需要处理的回调操作
        /*
        IO线程 mainloop accept fd <= channel subloop
        mainloop 事先注册一个callback（需要subloop来执行） 在subloop accept new connection后，执行这个callback
        */
        doPendingFunctors();
    }

    LOG_INFO("EventLoop %p stop looping \n", this);
    looping_ = false;
}

// 退出事件循环 1.loop所在的线程调用quit 2.在其他非loop线程中调用loop的quit方法
/*
                mainloop

        ========================= 生产者-消费者的线程安全队列(muduo没有使用)
    
    subloop1  subloop2  subloop3  subloop4
*/
void EventLoop::quit()
{
    quit_ = true;

    if(!isInLoopThread()) // 如果是在其它线程调用quit，例如在一个subloop（woker）中调用mainloop（IO）的quit，则需要唤醒mainloop（IO）
    {
        wakeup();
    }
}

// 在当前loop中执行cb
void EventLoop::runInLoop(Functor cb)
{
    if(isInLoopThread()) // 在当前的loop线程中执行cb
    {
        cb();
    }
    else // 在非当前loop线程中执行cb，则需要唤醒loop所在的线程，执行cb
    {
        queueInLoop(cb);
    }
}

void EventLoop::queueInLoop(Functor cb)
{
    {
        std::unique_lock<std::mutex> lock(mutex_);// 加锁，防止多个线程同时操作pendingFunctors_容器
        pendingFunctors_.emplace_back(cb);
    } // 解锁

    // 唤醒相应的需要执行上面回调操作的loop的线程，执行cb
    // 这里的callingPendingFunctors_的意思是：当前loop正在执行回调函数，但是有新的回调函数加入到pendingFunctors_中，则需要唤醒loop所在的线程
    if(!isInLoopThread() || callingPendingFunctors_) 
    
    {
        wakeup(); // 唤醒loop所在的线程
    }
}


void EventLoop::handleRead()
{
    uint64_t one = 1;
    ssize_t n = read(wakeupFd_, &one, sizeof one);
    if(n != sizeof one)
    {
        LOG_ERROR("EventLoop::handleRead() reads %ld bytes instead of 8 \n", n);
    }
}

// 唤醒loop所在的线程，向wakeupFd_写入数据，wakeupChannel就发生读事件，当前loop线程就会被唤醒
void EventLoop::wakeup()
{
    uint64_t one = 1; 
    ssize_t n = write(wakeupFd_, &one, sizeof one);
    if(n != sizeof one)
    {
        LOG_ERROR("EventLoop::wakeup() writes %ld bytes instead of 8 \n", n);
    }
}

void EventLoop::updateChannel(Channel* channel)
{
    poller_->updateChannel(channel);
}
void EventLoop::removeChannel(Channel* channel)
{
    poller_->removeChannel(channel);
}
bool EventLoop::hasChannel(Channel* channel)
{
    return poller_->hasChannel(channel);
}

void EventLoop::doPendingFunctors()
{
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;

    {
        std::unique_lock<std::mutex> lock(mutex_);
        // 交换两个容器，避免在执行回调函数时，有新的回调函数加入到pendingFunctors_中，这种设计值得学习
        functors.swap(pendingFunctors_);
    }

    for(const Functor& functor : functors)
    {
        functor(); // 执行当前loop需要执行的回调函数
    }
    callingPendingFunctors_ = false;
}