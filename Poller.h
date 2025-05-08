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
    virtual ~Poller() = default; // 虚析构函数，定义为虚是为了保证子类能够正确释放基类

    //给所有IO复用保留统一的接口，纯虚函数，定义为纯虚函数是为了保证子类必须实现该函数
    virtual Timestamp poll(int timeoutMs, ChannelList *activeChannels) = 0; // 轮询IO事件
    virtual void updateChannel(Channel *channel) = 0; // 更新channel的感兴趣的事件
    virtual void removeChannel(Channel *channel) = 0; // 移除channel

    //判断参数channel是否在当前Poller中
    virtual bool hasChannel(Channel *channel) const;

    //EventLoop可以通过该接口获取默认的IO复用的具体实现，不能在Poller.cc中实现，因为Poller是基类，不能依赖于具体的IO复用方式，需要单独实现
    static Poller* newDefaultPoller(EventLoop *loop);

protected:
    // Map的key是sockfd，value是sockfd所属的channel通道类型
    using ChannelMap = std::unordered_map<int,Channel*>;
    ChannelMap channels_;
private:
    EventLoop* ownerLoop_; //定义Poller所属的事件循环
};