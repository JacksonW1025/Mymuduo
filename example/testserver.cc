#include <mymuduo/TcpServer.h>
#include <mymuduo/Logger.h>
#include <string>
#include <functional>
class EchoServer
{
public:
    EchoServer(EventLoop *loop, 
            const InetAddress &listenAddr, 
            const std::string &nameArg)
            : loop_(loop),
            server_(loop, listenAddr, nameArg)
    {
        // 注册回调函数
        server_.setConnectionCallback(
            std::bind(&EchoServer::onConnection, this, std::placeholders::_1));

        server_.setMessageCallback(
            std::bind(&EchoServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

        // 设置合适的loop线程数量 loop thread pool 的线程数
        server_.setThreadNum(3);
    }
    void start()
    {
        server_.start();
    }
    
private:
    // 连接建立或者断开的回调
    void onConnection(const TcpConnectionPtr &conn)
    {
        if (conn->connected())
        {
            LOG_INFO("Connection UP: %s", conn->peerAddress().toIpPort().c_str());
        }
        else
        {
            LOG_INFO("Connection DOWN: %s", conn->peerAddress().toIpPort().c_str());
        }
    }
    // 可读事件回调
    void onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time)
    {
        std::string msg = buf->retrieveAsString();
        conn->send(msg);
        conn->shutdown(); // 写端关闭，EPOLLHUP->closeCallback_
    }
    EventLoop *loop_;
    TcpServer server_;
};
int main() 
{
    EventLoop loop;
    InetAddress addr(8000);
    EchoServer server(&loop, addr, "EchoServer-01"); // Acceptor non-blocking, listen socket
    server.start(); // listen loop thread  listenfd =>acceptChannel => mainLoop
    loop.loop(); // 启动mainLoop 的底层Poller（epoll）
    return 0;
}