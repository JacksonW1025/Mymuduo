#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <string.h>
#include <errno.h>

// 设置文件描述符为非阻塞模式
void setNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int main() {
    // g++ -o epolldemo Epolldemo.cc
    // 创建 epoll 实例
    // EPOLL_CLOEXEC 标志确保在 exec 时关闭 epoll 实例
    int epollfd = epoll_create1(EPOLL_CLOEXEC);
    if (epollfd == -1) {
        std::cerr << "epoll_create1 failed: " << strerror(errno) << std::endl;
        return 1;
    }

    // 设置标准输入为非阻塞模式
    setNonBlocking(STDIN_FILENO);

    // 创建 epoll 事件结构体
    struct epoll_event ev;
    ev.events = EPOLLIN;  // 监听读事件
    ev.data.fd = STDIN_FILENO;  // 监听标准输入

    // 将标准输入添加到 epoll 实例中
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, STDIN_FILENO, &ev) == -1) {
        std::cerr << "epoll_ctl failed: " << strerror(errno) << std::endl;
        close(epollfd);
        return 1;
    }

    // 创建事件数组，用于接收就绪的事件
    const int MAX_EVENTS = 10;
    struct epoll_event events[MAX_EVENTS];

    std::cout << "开始监听标准输入，输入 'quit' 退出程序" << std::endl;

    // 主事件循环
    while (true) {
        // 等待事件发生，超时时间设为 -1，表示永久等待
        int nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        if (nfds == -1) {
            std::cerr << "epoll_wait failed: " << strerror(errno) << std::endl;
            break;
        }

        // 处理所有就绪的事件
        for (int i = 0; i < nfds; ++i) {
            if (events[i].data.fd == STDIN_FILENO) {
                // 读取标准输入
                char buffer[1024];
                ssize_t n = read(STDIN_FILENO, buffer, sizeof(buffer) - 1);
                
                if (n > 0) {
                    buffer[n] = '\0';
                    std::cout << "收到输入: " << buffer;
                    
                    // 检查是否要退出
                    if (strncmp(buffer, "quit", 4) == 0) {
                        std::cout << "程序退出" << std::endl;
                        close(epollfd);
                        return 0;
                    }
                } else if (n == -1) {
                    if (errno != EAGAIN) {
                        std::cerr << "read error: " << strerror(errno) << std::endl;
                    }
                }
            }
        }
    }

    // 清理资源
    close(epollfd);
    return 0;
}
