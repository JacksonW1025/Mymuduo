#pragma once
#include "noncopyable.h"
#include "Channel.h"
#include "Timestamp.h"
#include <vector>
#include <unordered_map>

class Channel;
class EventLoop;

// muduo库中多路事件分发器的核心IO复用模块
class Poller : noncopyable
{
public:
    using ChannelList = std::vector<Channel*>;

    Poller(EventLoop *loop);
    virtual ~Poller();
protected:
    using ChannelMap = std::unordered_map<int,Channel*>;
    ChannelMap channels_;
private:
    EventLoop* ownerLoop_; //定义Poller所属的事件循环
}