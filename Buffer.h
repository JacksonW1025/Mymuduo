#pragma once

#include <vector>
#include <string>
#include <algorithm>

// 网络库底层的缓冲区
class Buffer
{
public:
    static const size_t kCheapPrepend = 8;
    static const size_t kInitialSize = 1024;

    explicit Buffer(size_t initialSize = kInitialSize)
        : buffer_(kCheapPrepend + initialSize)
        , readerIndex_(kCheapPrepend)
        , writerIndex_(kCheapPrepend)
    {}

    size_t readableBytes() const { return writerIndex_ - readerIndex_; }
    size_t writableBytes() const { return buffer_.size() - writerIndex_; }
    size_t prependableBytes() const { return readerIndex_; }

    // 返回缓冲区中可读数据的起始地址
    const char *peek() const { return begin() + readerIndex_; }
    char *peek() { return begin() + readerIndex_; }

    // OnMessage string <- Buffer
    void retrieve(size_t len)
    {
        if (len < readableBytes())
        {
            readerIndex_ += len; // 应用只读取了缓冲区数据的一部分，就是len，还剩下readerIndex_ += len - writerIndex_
        }
        else // len == readableBytes()
        {
            retrieveAll();
        }
    }

    void retrieveAll()
    {
        readerIndex_ = kCheapPrepend;
        writerIndex_ = kCheapPrepend;
    }

    // 把onMessage函数上报的string数据，转换成string类型的数据返回
    std::string retrieveAsString()
    {
        return retrieveAsString(readableBytes());// 应用可读取数据的长度
    }

    std::string retrieveAsString(size_t len)
    {
        std::string result(peek(), len);
        retrieve(len); // 缓冲区中可读取数据的长度减少，对缓冲区进行复位
        return result;
    }

    // buffer_.size() - writerIndex_ >= len
    void ensureWritableBytes(size_t len)
    {
        if (writableBytes() < len)
        {
            makeSpace(len); // 扩容
        }
    }
    
    // 把[data, data + len]内存中的数据添加到缓冲区中
    void append(const char *data, size_t len)
    {
        ensureWritableBytes(len);
        std::copy(data, data + len, beginWrite());
        writerIndex_ += len;
    }

    char* beginWrite()
    {
        return begin() + writerIndex_;
    }

    const char* beginWrite() const
    {
        return begin() + writerIndex_;
    }
    
    // 从fd上读取数据
    ssize_t readFd(int fd, int *saveErrno);

    // 从fd上发送数据
    ssize_t writeFd(int fd, int *saveErrno);

private:
    char *begin() { return &*buffer_.begin(); } // 获取缓冲区底层数组首元素地址
    const char *begin() const { return &*buffer_.begin(); } // const版本，用于const对象
    void makeSpace(size_t len)
    {
        /*
        扩容策略：
        1. 如果缓冲区剩余可写空间足够，则直接扩容
        2. 如果缓冲区剩余可写空间不足，则将缓冲区中的数据向前移动，腾出空间
        kCheapPrepend | reader | writer | 
        kCheapPrepend |      len            |
        */
        if (writableBytes() + prependableBytes() - kCheapPrepend < len)
        {
            buffer_.resize(writerIndex_ + len);
        }
        else
        {
            size_t readable = readableBytes();
            std::copy(begin() + readerIndex_, begin() + writerIndex_, begin() + kCheapPrepend);
            readerIndex_ = kCheapPrepend;
            writerIndex_ = readerIndex_ + readable;
        }
    }
    std::vector<char> buffer_;
    size_t readerIndex_;
    size_t writerIndex_;
}; 