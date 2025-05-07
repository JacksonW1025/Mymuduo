#include "Poller.h"
// 这是一个公共的源文件，避免Poller.cc去依赖子类，这是一个设计的细节
Poller* Poller::newDefaultPoller(EventLoop *loop)
{
    if(::getenv("MUDUO_USE_POLL"))
    {
        return nullptr; // 如果定义了MUDUO_USE_POLL，则不使用默认的IO复用方式epoll,使用poll
    }
    else{
        return nullptr; // 如果未定义MUDUO_USE_POLL，则使用默认的IO复用方式epoll,生成epoll实例
    }
}