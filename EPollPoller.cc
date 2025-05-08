#include "EPollPoller.h"
#include "Logger.h"
#include "Channel.h"
#include <errno.h>
#include <unistd.h>
// #include <cstring>
#include <strings.h> // bzero函数在strings.h头文件中

const int kNew = -1; // 表示channel未添加到epoll中，channel的成员index_默认是-1
const int kAdded = 1; // 表示channel已添加到epoll中
const int kDeleted = 2; // 表示channel已从epoll中删除

EPollPoller::EPollPoller(EventLoop *loop)
    : Poller(loop)
    , epollfd_(::epoll_create1(EPOLL_CLOEXEC)) // epoll_create1提供选项EPOLL_CLOEXEC，表示在创建epoll实例时，设置close-on-exec标志，当execve系统调用执行时，关闭epoll实例
    , events_(kInitEventListSize) // muduo库底层epoll事件列表vector<epoll_event>的默认长度是16
{
    if(epollfd_ < 0)
    {
        // 如果epoll_create1失败，则输出错误信息并退出
        LOG_FATAL("epoll_create error: %d \n", errno);
    }
}

EPollPoller::~EPollPoller()
{
    // 关闭epoll实例,close函数在unistd.h头文件中
    ::close(epollfd_);
}

// channel update remove => EventLoop updateChannel removeChannel => Poller updateChannel removeChannel => EPollPoller updateChannel removeChannel
/*
             EventLoop
              /      \
             /        \
        ChannelList   Poller
                        ChannelMap <fd, channel*>

*/

void EPollPoller::updateChannel(Channel *channel)
{
    // channel内有一个index_，表示channel在poller中的状态
    const int index = channel->index();
    LOG_INFO("func=%s => fd = %d events = %d index = %d\n", __FUNCTION__, channel->fd(), channel->events(), index);

    if(index == kNew || index == kDeleted){
        // 如果channel在poller中的状态是kNew或kDeleted，则将channel添加到epoll中
        int fd = channel->fd();
        if(index == kNew){
            channels_[fd] = channel;//加入ChannelMap
        }
        else{ // index == kDeleted
        }
        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD, channel);
    }
    else{ // index == kAdded，channel已经在poller上注册过了
        int fd = channel->fd();
        if(channel->isNoneEvent()){
            update(EPOLL_CTL_DEL, channel);
            channel->set_index(kDeleted);
        }
        else{
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

void EPollPoller::removeChannel(Channel *channel)
{
    int fd = channel->fd();
    channels_.erase(fd);

    LOG_INFO("func=%s => fd = %d\n", __FUNCTION__, fd);

    const int index = channel->index();
    if(index == kAdded){
        update(EPOLL_CTL_DEL, channel);
    }
    channel->set_index(kNew);
}

Timestamp EPollPoller::poll(int timeoutMs, ChannelList *activeChannels)
{
    // 实际上应该使用LOG_DEBUG更为合适
    LOG_INFO("func = %s => fd total count: %zu", __FUNCTION__, channels_.size());

    //&*events_.begin() 获取events_的第一个元素的地址,static_cast<int>是C++类型安全的类型转换
    // epoll_wait返回值:成功返回就绪事件的数量,失败返回-1
    int numEvents = ::epoll_wait(epollfd_, &*events_.begin(), static_cast<int>(events_.size()), timeoutMs);
    int saveErrno = errno;
    Timestamp now(Timestamp::now());

    if(numEvents > 0){
        LOG_INFO("%d events happened\n", numEvents);
        fillActiveChannels(numEvents, activeChannels);
        // 如果epoll_wait返回的事件数量等于events_的容量，则扩容，扩容为原来的2倍；这里是LT模式
        if(numEvents == static_cast<int>(events_.size())){
            events_.resize(events_.size() * 2);
        }
    }
    else if(numEvents == 0){
        LOG_DEBUG("%s timeout!\n", __FUNCTION__);
    }
    else{
        if(saveErrno != EINTR){
            errno = saveErrno; // 避免期间受到其他信号的干扰
            LOG_ERROR("EPollPoller::poll() error: %d\n", saveErrno);
        }
    }
    return now;
}

// 这个函数用于更新channel在epoll中的状态
void EPollPoller::update(int operation, Channel *channel)
{
    epoll_event event;
    // memset(&event, 0, sizeof event);
    bzero(&event, sizeof event);
    int fd = channel->fd();

    event.events = channel->events(); // 设置事件类型
    event.data.fd = fd;
    event.data.ptr = channel;

    if(::epoll_ctl(epollfd_, operation, fd, &event) < 0){
        if(operation == EPOLL_CTL_DEL){
            LOG_ERROR("epoll_ctl del error: %d", errno);
        }
        else{
            LOG_FATAL("epoll_ctl add/mod error: %d", errno);
        }
    }
}

// 这个函数用于填充活跃的channel连接
void EPollPoller::fillActiveChannels(int numEvents, ChannelList *activeChannels) const
{
    for(int i = 0; i < numEvents; ++i){
        Channel *channel = static_cast<Channel*>(events_[i].data.ptr); // 获取事件对应的channel
        channel->set_revents(events_[i].events); // 设置channel的revents_
        activeChannels->push_back(channel); // EventLoop就拿到了它的poller返回的所有发生事件的channel列表
    }
}
