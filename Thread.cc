#include "Thread.h"
#include "CurrentThread.h"

#include <semaphore.h>
std::atomic_int Thread::numCreated_(0); // 初始化静态成员变量

Thread::Thread(ThreadFunc func, const std::string &name = std::string())
    : started_(false),
      joined_(false),
      tid_(0),
      func_(std::move(func)),
      name_(name)
{
    setDefaultName();
}

Thread::~Thread()
{
    if(started_ && !joined_) // join是等待线程结束，detach是设置分离线程，线程结束后自动释放资源
    {
        thread_->detach(); // 设置分离线程，线程结束后自动释放资源
        // thread类提供了detach方法，用于设置分离线程，线程结束后自动释放资源
    }
}

void Thread::start() // 一个Thread对象，记录的就是一个新线程的详细信息
{
    started_ = true;
    sem_t sem;
    sem_init(&sem, false, 0);

    // 创建线程，使用了lambda表达式，捕获了当前线程的tid_和func_，&表示捕获当前线程的tid_和func_
    thread_ = std::shared_ptr<std::thread>(new std::thread([&]() {
        tid_ = CurrentThread::tid(); // 获取当前线程的tid
        sem_post(&sem); // 通知主线程，线程已经启动
        func_(); // 执行线程函数
    }));

    // 这里必须等待获取上面的tid_
    sem_wait(&sem);
}
void Thread::join()
{
    joined_ = true;
    thread_->join();
}

void Thread::setDefaultName()
{
    int num = ++numCreated_;

    if(name_.empty())
    {
        char buf[32] = {0};
        snprintf(buf, sizeof buf, "Thread%d", num);
        name_ = buf;
    }
}