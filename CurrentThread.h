#pragma once
#include <unistd.h>
#include <sys/syscall.h>

namespace CurrentThread
{
    // 定义一个__thread变量，用于缓存当前线程的tid值,__thread是GCC的扩展，用于定义线程局部存储的变量,
    // 每个线程都有自己的t_cachedTid变量，互不干扰
    extern __thread int t_cachedTid;

    void cacheTid();
    
    inline int tid() // 内联函数，可以写在头文件中
    {
        if(__builtin_expect(t_cachedTid == 0, 0)){
            //__builtin_expect(EXP, N) 是GCC的扩展，用于提示编译器EXP==N的可能性很大
            cacheTid();
        }
        return t_cachedTid;
    }
}