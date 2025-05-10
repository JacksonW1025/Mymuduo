# Mymuduo
手写简化muduo网络库项目，作个人源码阅读学习用途，代码注释完全。

## Design Patterns
muduo库主要采用了以下几种设计模式：

1. **Reactor模式**
   - 这是muduo最核心的设计模式
   - 通过`EventLoop`、`Channel`和`Poller`三个核心类实现
   - 采用"one loop per thread"的设计理念，每个线程运行一个事件循环
   - 主要组件：
     - `EventLoop`: 事件循环，负责事件分发
     - `Channel`: 通道，封装了文件描述符和事件回调
     - `Poller`: 多路事件分发器，负责IO复用

2. **观察者模式**
   - 在事件处理中大量使用
   - `Channel`作为被观察者，当事件发生时通知观察者（回调函数）
   - 通过回调函数（`EventCallback`）实现事件处理

3. **工厂模式**
   - 用于创建`Poller`对象
   - 通过`Poller::newDefaultPoller()`静态工厂方法创建具体的`Poller`实现

4. **单例模式**
   - 用于管理全局资源
   - 比如`Logger`类就是单例模式

5. **组合模式**
   - 在`TcpServer`和`EventLoop`等类中使用
   - 通过组合其他对象来实现复杂功能

6. **策略模式**
   - 在IO复用实现中体现
   - 可以灵活切换不同的IO复用方式（epoll、poll等）

7. **RAII模式**
   - 大量使用智能指针管理资源
   - 使用RAII确保资源的正确释放

8. **非拷贝模式**
   - 通过继承`noncopyable`类实现
   - 防止对象被意外拷贝

这些设计模式的组合使用使得muduo库具有：
- 高并发性能
- 良好的可扩展性
- 清晰的代码结构
- 易于维护和扩展

其中Reactor模式是最核心的设计模式，它使得muduo能够高效地处理大量并发连接。
