#include "CurrentThread.h"

#include <stdio.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

namespace muduo {
namespace detail {
    pid_t gettid()
    {
        return static_cast<pid_t>(::syscall(SYS_gettid));
    }
} // namespace detail

namespace current_thread {
    thread_local int t_cacheTid = 0;
    thread_local char t_tidString[32];
    thread_local int t_tidStringLength = 6;
    thread_local const char* t_threadName = "unknown";

    void cacheTid()
    {
        if (t_cacheTid == 0) {
            t_cacheTid = detail::gettid();
            t_tidStringLength = ::snprintf(t_tidString, sizeof(t_tidString), "%5d ", t_cacheTid);
        }
    }

} // namespace current_thread

} // namespace muduo
