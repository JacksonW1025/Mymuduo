#pragma once

#include "Socket.h"
#include "Channel.h"
#include "InetAddress.h"
#include "noncopyable.h"
class EventLoop;
class InetAddress;
class Acceptor : noncopyable
{
public:
    using NewConnectionCallback = std::function<void(int sockfd, const InetAddress &)>;
    Acceptor(EventLoop *loop, const InetAddress &listenAddr, bool reuseport);
    ~Acceptor();

    void setNewConnectionCallback(const NewConnectionCallback &cb)
    {
        newConnectionCallback_ = std::move(cb); //可以用std::move(cb)，把cb的所有权转移给newConnectionCallback_
    }
    void listen();

private:
    void handleRead();

    EventLoop *loop_;
    Socket acceptSocket_;
    Channel acceptChannel_;
    NewConnectionCallback newConnectionCallback_;
    bool listenning_;
};