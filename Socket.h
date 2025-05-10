#pragma once

#include "noncopyable.h"

class InetAddress;
// 封装socket fd
class Socket : noncopyable
{
public:
    explicit Socket(int sockfd)
        : sockfd_(sockfd)
    {}

    ~Socket();

    int fd() const { return sockfd_; }
    void bindAddress(const InetAddress &localaddr); // 绑定地址
    void listen(); // 监听
    int accept(InetAddress *peeraddr); // 接受连接
    void shutdownWrite(); // 关闭写端
    void setTcpNoDelay(bool on); // 设置是否开启Nagle算法
    void setReuseAddr(bool on); // 设置是否重用地址
    void setReusePort(bool on); // 设置是否重用端口
    void setKeepAlive(bool on); // 设置是否开启心跳检测

private:    
    const int sockfd_;
};
