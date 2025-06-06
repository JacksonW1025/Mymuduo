#include "CurrentThread.h"

namespace CurrentThread
{
    
    __thread int t_cachedTid = 0;

    void cacheTid()
    {
        if(t_cachedTid == 0){
            //通过Linux系统调用获取当前线程的tid值
            t_cachedTid = static_cast<pid_t>(::syscall(SYS_gettid)); // 获取当前线程的tid，使用的方法是syscall并传入SYS_gettid，返回值是pid_t类型
        }
    }
}