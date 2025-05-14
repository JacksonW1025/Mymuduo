#include "Channel.h"
#include <sys/epoll.h>
#include "EventLoop.h"
#include "Logger.h" 

const int Channel::kNoneEvent = 0; //无事件
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI; //读事件
const int Channel::kWriteEvent = EPOLLOUT; //写事件

//本手写的muduo库只支持epoll，不同于原版支持poll和epoll

//EventLoop: ChannelList Poller
//省略了eventHandling_,以及addedToLoop_，因为他们只用于调试断言
Channel::Channel(EventLoop *loop, int fd)
    : loop_(loop)
    , fd_(fd)
    , events_(0)
    , revents_(0)
    , index_(-1) //表示channel在poller中的状态
    , tied_(false)
    {}

Channel::~Channel()
{
    // assert(!eventHandling_); //如果Channel对象被销毁，说明没有事件在处理
    // assert(!addedToLoop_); //如果Channel对象被销毁，说明没有添加到poller中
    // if(loop_->isInLoopThread()) //如果当前线程是事件循环线程
    // {
    //     assert(!loop_->hasChannel(this)); //如果当前线程是事件循环线程，说明没有添加到poller中
    // }
}

// channel的tie方法什么时候调用过？ 一个新的Tcp连接创建的时候，TcpConnection => Channel => TcpConnection::connectEstablished
void Channel::tie(const std::shared_ptr<void> &obj)
{
    tie_ = obj; //将obj赋值给tie_
    tied_ = true; //设置tied_为true
}

//当改变channel所表示fd的event事件后，update负责在poller里面更改fd相应的事件epoll_ctl
//EventLoop => ChannelList Poller
void Channel::update()
{
    // 通过channel所属的EventLoop，调用poller的相应方法，注册fd的events事件
    loop_->updateChannel(this); //将当前channel添加到poller中
}

// 在channel所属的eventloop中，删除当前的channel，eventloop里有一个channel列表
void Channel::remove()
{
    loop_->removeChannel(this); //从poller中删除当前channel
}

void Channel::handleEvent(Timestamp receiveTime)
{
    // 按tied_的值判断的原因是为了防止在Channel对象被销毁后，回调函数还在执行
    if(tied_)
    {
        std::shared_ptr<void> guard = tie_.lock(); //获取tie_的锁,提升弱指针为强指针
        if(guard) //如果锁获取成功
        {
            handleEventWithGuard(receiveTime); //调用handleEventWithGuard函数
        }
    }
    else
    {
        handleEventWithGuard(receiveTime); //调用handleEventWithGuard函数
    }
}

//根据poller通知的channel发生的具体事件，由channel来调用具体的事件回调函数
void Channel::handleEventWithGuard(Timestamp receiveTime)
{
    LOG_INFO("channel handleEvent revents = %d\n", revents_); //打印事件发生的类型

   if((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)) //如果发生了挂断事件，并且没有读事件
   {
       if(closeCallback_) //如果关闭事件回调函数不为空
       {
           closeCallback_(); //调用关闭事件回调函数
       }
   }

   if(revents_ & EPOLLERR) //如果发生了错误事件
   {
       if(errorCallback_) //如果错误事件回调函数不为空
       {
           errorCallback_(); //调用错误事件回调函数
       }
   }

   if(revents_ & (EPOLLIN | EPOLLPRI)) //如果发生了读事件
   {
       if(readCallback_) //如果读事件回调函数不为空
       {
           readCallback_(receiveTime); //调用读事件回调函数
       }
   }

   if(revents_ & EPOLLOUT) //如果发生了写事件
   {
       if(writeCallback_) //如果写事件回调函数不为空
       {
           writeCallback_(); //调用写事件回调函数
       }
   }
}