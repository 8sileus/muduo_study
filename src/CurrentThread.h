#ifndef __MUDUO_CURRENTTHREAD_H__
#define __MUDUO_CURRENTTHREAD_H__

namespace muduo {
namespace current_thread {

    extern thread_local int t_cacheTid;
    extern thread_local char t_tidString[32];
    extern thread_local int t_tidStringLength;
    extern thread_local const char* t_threadName;

    void cacheTid();

    inline int tid()
    {
        if (__builtin_expect(t_cacheTid == 0, 0)) {
            cacheTid();
        }
        return t_cacheTid;
    }

    inline const char* tidString() // for logging
    {
        return t_tidString;
    }

    inline int tidStringLength() // for logging
    {
        return t_tidStringLength;
    }

    inline const char* name()
    {
        return t_threadName;
    }

} // namespace current_thread
} // namespace muduo

#endif //__MUDUO_CURRENTTHREAD_H__