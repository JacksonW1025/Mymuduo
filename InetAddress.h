#pragma once

#include <netinet/in.h>
#include <string>

// 封装socket地址的类,Muduo库中它继承自copyable类，允许拷贝，这里进行了简化
class InetAddress
{
public:
    explicit InetAddress(uint16_t port = 0, std::string ip = "127.0.0.1"); // 构造函数，默认IP地址
    explicit InetAddress(const struct sockaddr_in& addr) // 构造函数，传入sockaddr_in结构体
        : addr_(addr) {} // 初始化成员变量addr_

    std::string toIp() const; // 转换为IP地址字符串
    std::string toIpPort() const; // 转换为IP地址和端口号字符串
    uint16_t toPort() const; // 获取端口号

    const sockaddr_in* getSockAddr() const // 获取sockaddr结构体的指针
    {
        return &addr_; // 返回sockaddr结构体的地址
    }
    void setSockAddr(const sockaddr_in& addr)
    {
        addr_ = addr;
    }
    
private:
    sockaddr_in addr_; // socket地址结构体,其中sockaddr_in是一个结构体，包含了IP地址和端口号等信息，来自<netinet/in.h>头文件


};