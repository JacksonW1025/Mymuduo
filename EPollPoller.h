#pragma once
#include "Poller.h"
#include "Timestamp.h"
#include <vector>
#include <sys/epoll.h> // 使用epoll的系统调用

class Channel;
class EventLoop;

/*
    epoll的使用
    epoll_create
    epoll_ctl add/mod/del
    epoll_wait
*/

class EPollPoller : public Poller
{
public:
    EPollPoller(EventLoop *loop);
    ~EPollPoller() override; //override是重写父类中的虚函数,编译器会检查，是C++11的特性

    //重写父类Poller中的虚函数
    Timestamp poll(int timeoutMs, ChannelList *activeChannels) override;
    void updateChannel(Channel *channel) override;
    void removeChannel(Channel *channel) override;
    // bool hasChannel(Channel *channel) const override;
  
private:
    static const int kInitEventListSize = 16;

    // 填充活跃的channel连接
    void fillActiveChannels(int numEvents, ChannelList *activeChannels) const;
    // 更新channel通道
    void update(int operation, Channel *channel);

    using EventList = std::vector<epoll_event>;
    int epollfd_; // epoll的文件描述符
    EventList events_; // 保存epoll的返回事件

    
};