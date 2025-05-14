#pragma once
#include "Buffer.h"
#include "noncopyable.h"
#include "InetAddress.h"
#include "Callbacks.h"
#include <memory>
#include <atomic> // 原子操作
#include <string>



class EventLoop;
class Socket;
class Channel;


/*
TcpServer => Acceptor => 有一个新用户连接，通过accept函数拿到connfd
TcpServer => TcpConnection 设置回调 => 给Channel设置回调 => Poller => Channel::handleEvent() => 调用TcpConnection::handleRead, handleWrite, handleClose, handleError
一个TcpConnection对应一个Socket和一个Channel
一个TcpConnection包含一个输入缓冲区和输出缓冲区
*/
class TcpConnection : noncopyable, public std::enable_shared_from_this<TcpConnection>
{
public:
    enum StateE {kDisconnected, kConnecting, kConnected, kDisconnecting};

    TcpConnection(EventLoop *loop, const std::string &name, int sockfd, const InetAddress &localAddr, const InetAddress &peerAddr);
    ~TcpConnection();

    EventLoop* getLoop() const { return loop_; }
    const std::string& name() const { return name_; }
    const InetAddress& localAddress() const { return localAddr_; }
    const InetAddress& peerAddress() const { return peerAddr_; }

    bool connected() const { return state_ == kConnected; }
    // bool disconnected() const { return state_ == kDisconnected; }
    
    void send(const std::string &buf);

    void shutdown();

    void setConnectionCallback(const ConnectionCallback &cb) { connectionCallback_ = cb; }
    void setMessageCallback(const MessageCallback &cb) { messageCallback_ = cb; }
    void setWriteCompleteCallback(const WriteCompleteCallback &cb) { writeCompleteCallback_ = cb; }
    void setHighWaterMarkCallback(const HighWaterMarkCallback &cb) { highWaterMarkCallback_ = cb; }
    void setCloseCallback(const CloseCallback &cb) { closeCallback_ = cb; }

    // 连接建立
    void connectEstablished();
    // 连接销毁
    void connectDestroyed();

    void setState(StateE state) { state_ = state; }
    
private:
    void handleRead(Timestamp receiveTime);
    void handleWrite();
    void handleClose();
    void handleError();


    void sendInLoop(const void *message, size_t len);
    void shutdownInLoop();

    EventLoop *loop_; // 所属的eventloop，这里绝对不是baseLoop，因为TcpConnection都是在subLoop中管理
    const std::string name_; // 连接名称
    std::atomic<StateE> state_; // 连接状态
    std::atomic_bool reading_; // 是否正在读取数据

    // 这里和Acceptor类似，使用智能指针管理Socket和Channel Acceptor->mainloop, TcpConnection->subloop
    std::unique_ptr<Socket> socket_; // 连接套接字
    std::unique_ptr<Channel> channel_; // 连接通道

    const InetAddress localAddr_; // 本地地址
    const InetAddress peerAddr_; // 对端地址

    ConnectionCallback connectionCallback_; // 连接回调函数
    MessageCallback messageCallback_; // 消息回调函数
    WriteCompleteCallback writeCompleteCallback_; // 写完成回调函数
    HighWaterMarkCallback highWaterMarkCallback_; // 高水位回调函数
    CloseCallback closeCallback_; // 关闭回调函数
    size_t highWaterMark_; // 高水位标记

    Buffer inputBuffer_; // 输入缓冲区，用于接收数据
    Buffer outputBuffer_; // 输出缓冲区，用于发送数据
};
