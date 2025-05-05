#include "InetAddress.h"
#include <strings.h>// bzero函数的头文件
#include <arpa/inet.h> // inet_addr函数的头文件
#include <string.h>
#include <iostream>

// 构造函数，默认IP地址，默认参数只能在声明中使用
InetAddress::InetAddress(uint16_t port,std::string ip)
{
    bzero(&addr_, sizeof(addr_)); // 清空addr_结构体,bzero函数将addr_结构体的所有字节都设置为0
    addr_.sin_family = AF_INET; // 设置地址族为IPv4
    addr_.sin_port = htons(port); // 设置端口号，htons函数将主机字节序转换为网络字节序
    addr_.sin_addr.s_addr = inet_addr(ip.c_str()); // 设置IP地址，inet_addr函数将字符串转换为网络字节序的IP地址,来自<arpa/inet.h>头文件
} 

std::string InetAddress::toIp() const
{
   // addr_
   // 函数的作用是将IP地址转换为字符串格式
   char buf[64] = {0}; // 定义一个64字节的字符数组，用于存储IP地址字符串
   ::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof(buf)); // 将网络字节序的IP地址转换为字符串，::的作用是表示全局命名空间
   return std::string(buf); // 返回IP地址字符串
} 

std::string InetAddress::toIpPort() const
{
    // ip:port
    // 函数的作用是将IP地址和端口号转换为字符串格式
    char buf[64] = {0}; // 定义一个64字节的字符数组，用于存储IP地址字符串
    ::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof(buf)); // 将网络字节序的IP地址转换为字符串，::的作用是表示全局命名空间
    size_t end = strlen(buf); // 获取IP地址字符串的长度
    uint16_t port = ntohs(addr_.sin_port); // 将网络字节序的端口号转换为主机字节序
    sprintf(buf + end, ":%u", port); // 将端口号添加到IP地址字符串后面，sprintf函数的作用将格式化字符串写入buf中
    return std::string(buf); // 返回IP地址和端口号字符串
}

uint16_t InetAddress::toPort() const
{
    return ntohs(addr_.sin_port); // 将网络字节序的端口号转换为主机字节序并返回
}

int main()
{
    //在Ubuntu系统上编译时，使用g++ -std=c++11 -o InetAddress InetAddress.cc
    // 测试InetAddress类
    InetAddress addr(8080);
    
    std::cout << addr.toIp() << std::endl; // 输出IP地址
    std::cout << addr.toIpPort() << std::endl; // 输出IP地址和端口号
    std::cout << addr.toPort() << std::endl; // 输出端口号
    return 0;
}