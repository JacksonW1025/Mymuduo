#pragma once 

#include "noncopyable.h"
#include <functional>
#include "Timestamp.h" //包含时间戳类
#include <memory> //包含智能指针

class EventLoop;//前向声明，用于避免循环引用，同时避免包含头文件——导致编译时间过长，暴露实现细节

/*
 理清楚EventLoop和Channel还有Poller之间的关系，在Reactor模型中对应Demultiplex，one loop per thread
 Channel理解为通道，封装了sockfd和其感兴趣的event，如EPOLLIN、EPOLLOUT事件
 还绑定了poller返回的具体事件
*/

class Channel : noncopyable
{
public:
    // typedef std::function<void()> EventCallback; //事件回调函数类型
    //在 C++ 中，std::function<void()> 是一种通用的函数包装器，用来封装任何可以匹配 void() 签名的可调用对象。
    using EventCallback = std::function<void()>; //事件回调函数类型
    using ReadEventCallback = std::function<void(Timestamp)>; //读事件回调函数类型

    Channel(EventLoop *loop, int fd); 
    ~Channel();

    //fd得到poller通知以后，处理事件的函数
    void handleEvent(Timestamp receiveTime); //这里不能只用前向声明Timestamp，因为需要使用到它的成员函数

    //设置回调函数对象
    void setReadCallback(ReadEventCallback cb) { readCallback_ = std::move(cb); } //由于ReadEventCallback是一个函数对象，是右值，所以使用std::move来转移所有权
    void setWriteCallback(EventCallback cb) { writeCallback_ = std::move(cb); }
    void setCloseCallback(EventCallback cb) { closeCallback_ = std::move(cb); }
    void setErrorCallback(EventCallback cb) { errorCallback_ = std::move(cb); }

    //防止当channel被手动remove掉后，channel还在执行回调操作
    void tie(const std::shared_ptr<void> &); //形参的名称可以省略，省略后编译器会自动推导出类型

    int fd() const { return fd_; } //获取文件描述符
    int events() const { return events_; } //获取注册的事件
    void set_revents(int revt) { revents_ = revt; } //设置poller返回的事件,int返回值会warning

    //设置fd相应的事件状态
    void enableReading() { events_ |= kReadEvent; update(); } //启用读事件，｜=表示按位或操作
    void disableReading() { events_ &= ~kReadEvent; update(); } //禁用读事件，&=表示按位与操作，~表示按位取反
    void enableWriting() { events_ |= kWriteEvent; update(); } //启用写事件
    void disableWriting() { events_ &= ~kWriteEvent; update(); } //禁用写事件
    void disableAll() { events_ = kNoneEvent; update(); } //禁用所有事件

    //返回fd当前的事件状态
    bool isNoneEvent() const { return events_ == kNoneEvent; } //判断是否没有事件
    bool isWriting() const { return events_ & kWriteEvent; } //判断是否有写事件
    bool isReading() const { return events_ & kReadEvent; } //判断是否有读事件

    int index() const { return index_; } //获取在poller中的索引 
    void set_index(int idx) { index_ = idx; } //设置在poller中的索引

    EventLoop *ownerLoop() { return loop_; } //获取事件循环
    void remove(); //从poller中删除fd

private:

    void update(); //更新poller中的fd事件
    void handleEventWithGuard(Timestamp receiveTime); //处理事件的函数，带锁

    static const int kNoneEvent; //无事件
    static const int kReadEvent; //读事件
    static const int kWriteEvent; //写事件

    EventLoop *loop_; //事件循环
    const int fd_; //文件描述符
    int events_; //注册fd感兴趣的事件
    int revents_; //poller返回的具体发生的事件
    int index_; //在poller中的索引

    std::weak_ptr<void> tie_; //weak_ptr是弱引用，作用是避免循环引用，不增加引用计数
    bool tied_; //是否绑定了一个对象

    //因为channel通道里面能够获知fd最终发生的事件，所以它负责调用具体的事件回调函数
    ReadEventCallback readCallback_; //读事件回调函数
    EventCallback writeCallback_; //写事件回调函数
    EventCallback closeCallback_; //关闭事件回调函数
    EventCallback errorCallback_; //错误事件回调函数
};