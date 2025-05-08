#pragma once 
#include "noncopyable.h"
#include "Timestamp.h"
#include "CurrentThread.h"
#include <functional>
#include <vector>
#include <atomic>
#include <memory> //智能指针
#include <mutex> //互斥锁

class Channel;  
class Poller;

//对外的事件循环编程使用的类,主要包含两个大模块，Channel和Poller（epoll的抽象）
class EventLoop : noncopyable
{
public:
    using Functor = std::function<void()>;

    EventLoop();
    ~EventLoop();

    // 开启事件循环
    void loop();
    // 退出事件循环
    void quit();

    Timestamp pollReturnTime() const { return pollReturnTime_; }

    // 在当前loop中执行cb
    void runInLoop(Functor cb);
    // 在当前loop中延迟执行cb
    void queueInLoop(Functor cb);

    // 唤醒loop所在的线程
    void wakeup();

    // EventLoop的方法 => Poller的方法
    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);
    bool hasChannel(Channel* channel);  

    // 判断当前线程是否是当前loop所在的线程
    bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); } 
    
private:
    void handleRead(); // 处理唤醒事件
    void doPendingFunctors(); // 执行回调函数

    using ChannelList = std::vector<Channel*>;

    std::atomic_bool looping_; // atomic_bool是原子类型，用于表示一个布尔值，原子操作是指在多线程环境下，对一个变量的操作不会被其他线程打断，保证了操作的线程安全性
    // 底层是通过CAS操作实现的，CAS操作是compare and swap的缩写，是一种原子操作，用于实现多线程环境下的互斥访问
    std::atomic_bool quit_; // 用于表示是否退出事件循环

    const pid_t threadId_; // 用于表示当前loop所在线程的id

    Timestamp pollReturnTime_; // 用于表示poll的返回时间

    std::unique_ptr<Poller> poller_; // 用于表示poller的智能指针

    int wakeupFd_; // 用于表示唤醒事件的文件描述符，当mainloop获取一个新用户的channel，通过轮询算法选择一个subloop，通过该成员唤醒subloop，处理channel。
    // 唤醒的方式是eventfd，eventfd是Linux内核提供的一种事件通知机制，通过一个文件描述符来通知事件的发生，当事件发生时，通过write操作向该文件描述符写入数据，当读取该文件描述符时，会返回0，从而触发事件处理。
    std::unique_ptr<Channel> wakeupChannel_; // 用于表示唤醒事件的channel的智能指针

    ChannelList activeChannels_; // 用于存储当前事件循环中需要处理的channel
    Channel* currentActiveChannel_; // 用于表示当前正在处理的channel

    std::atomic_bool callingPendingFunctors_; // 用于表示当前loop是否有需要执行的回调函数
    std::vector<Functor> pendingFunctors_; // 用于存储loop需要执行的回调函数
    std::mutex mutex_; // 用于保护pendingFunctors_容器的线程安全操作的互斥锁
};