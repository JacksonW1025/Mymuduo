#include "Buffer.h"
#include <errno.h>
#include <sys/uio.h>

/*
readFd函数： 
1. 从fd上读取数据，并存储到缓冲区中 Poller工作在LT模式
Buffer缓冲区是有大小的，但是从fd上读数据的时候，不知道tcp数据最终的大小
2. 如果缓冲区中剩余可写空间足够，则直接存储
3. 如果缓冲区中剩余可写空间不足，则将缓冲区中的数据向前移动，腾出空间
4. 如果缓冲区中剩余可写空间足够，则直接存储
5. 如果缓冲区中剩余可写空间不足，则将缓冲区中的数据向前移动，腾出空间
*/
ssize_t Buffer::readFd(int fd, int *saveErrno)
{
    char extrabuf[65536] = {0}; // 栈上的内存空间

    struct iovec vec[2];

    const size_t writable = writableBytes(); // 这是buffer底层缓冲区剩余可写空间的大小
    vec[0].iov_base = begin() + writerIndex_;
    vec[0].iov_len = writable;

    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof(extrabuf);

    const int iovcnt = (writable < sizeof(extrabuf)) ? 2 : 1;
    // readv 函数： 将数据从fd读取到多个缓冲区中
    const ssize_t n = ::readv(fd, vec, iovcnt);
    if (n < 0)
    {
        *saveErrno = errno;
    }
    else if (n <= writable) // Buffer的可写空间足够
    {
        writerIndex_ += n;
    }
    else // extrabuf中存储了剩余的数据
    {
        writerIndex_ = buffer_.size();
        append(extrabuf, n - writable); // writerIndex_ 开始写 n - writable 个字节
    }
    return n;
}
    