#ifndef __MUDUO_EVENTLOOPTHREAD_H__
#define __MUDUO_EVENTLOOPTHREAD_H__

#include "Thread.h"

#include <condition_variable>
#include <functional>
#include <mutex>
#include <string>

namespace muduo {

class EventLoop;

class EventLoopThread : Noncopyable {
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback(),
        const std::string& name = std::string());
    ~EventLoopThread();

    EventLoopThread(EventLoopThread&&) = default;
    EventLoopThread& operator=(EventLoopThread&&) = default;

    EventLoop* startLoop();

private:
    void threadFunc();

private:
    EventLoop* loop_;
    bool exiting_;
    Thread thread_;
    std::mutex mutex_;
    std::condition_variable cond_;
    ThreadInitCallback callback_;
};

} // namespace muduo

#endif //__MUDUO_EVENTLOOPTHREAD_H__