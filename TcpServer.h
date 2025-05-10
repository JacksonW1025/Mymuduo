#pragma once
#include "noncopyable.h"


/*
用户使用muduo库建立服务器程序
*/
#include "EventLoop.h"
#include "EventLoopThreadPool.h"
#include "InetAddress.h"
#include "Acceptor.h"
#include "Callbacks.h"

#include <functional>
#include <atomic>
#include <unordered_map>

//对外的服务器编程使用的类
class TcpServer : noncopyable
{
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    enum Option
    {
        kNoReusePort,
        kReusePort,
    };

    TcpServer(EventLoop *loop,
             const InetAddress &listenAddr,
             Option option = kReusePort);

    ~TcpServer();

    void setThreadInitCallback(const ThreadInitCallback &cb) { threadInitCallback_ = cb; }

    void setConnectionCallback(const ConnectionCallback &cb) { connectionCallback_ = cb; }

    void setMessageCallback(const MessageCallback &cb) { messageCallback_ = cb; }

    void setWriteCompleteCallback(const WriteCompleteCallback &cb) { writeCompleteCallback_ = cb; }

    //设置底层subloop的个数
    void setThreadNum(int numThreads);

    //开启服务器监听
    void start();

    //设置连接回调
    
    
private:
    //当有新连接时，调用此函数  
    void newConnection(int sockfd, const InetAddress &peerAddr);

    //当有连接断开时，调用此函数
    void removeConnection(const TcpConnectionPtr &conn);    

    //在subloop中调用
    void removeConnectionInLoop(const TcpConnectionPtr &conn);

    using ConnectionMap = std::unordered_map<std::string, TcpConnectionPtr>;

    EventLoop *loop_; // 服务器的主loop,baseLoop,用户定义的loop
    const std::string ipPort_; // 服务器ip和端口
    const std::string name_; // 服务器名称
    std::unique_ptr<Acceptor> acceptor_; // 运行在main loop，接受新连接
    std::shared_ptr<EventLoopThreadPool> threadPool_; // 线程池，one loop per thread
    ConnectionCallback connectionCallback_; // 新连接回调
    MessageCallback messageCallback_; // 读写消息回调
    WriteCompleteCallback writeCompleteCallback_; // 消息写完成回调

    ThreadInitCallback threadInitCallback_; // loop线程初始化回调
    
    std::atomic_int started_; // 是否启动

    int nextConnId_; // 下一个连接id
    ConnectionMap connections_; // 保存所有连接
};

