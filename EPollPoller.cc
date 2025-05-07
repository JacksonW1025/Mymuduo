#include "EPollPoller.h"
#include "Logger.h"
#include <errno.h>

const int kNew = -1; // 表示channel未添加到epoll中
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
    // 关闭epoll实例
    ::close(epollfd_);
}


