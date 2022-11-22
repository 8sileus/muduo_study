#ifndef __MUDUO_EVENTLOOPTHREADPOOL_H__
#define __MUDUO_EVENTLOOPTHREADPOOL_H__

#include "Noncopyable.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace muduo {

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : Noncopyable {
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    EventLoopThreadPool(EventLoop* baseloop, const std::string& nameArg);
    ~EventLoopThreadPool();

    void setThreadNum(int numThreads) { numThreads_ = numThreads; }
    void start(const ThreadInitCallback& cb = ThreadInitCallback());

    EventLoop* getNextLoop();
    std::vector<EventLoop*> getAllLoops();

    bool started() const { return started_; }
    const std::string& name() const { return name_; }

private:
    EventLoop* baseLoop_;
    std::string name_;
    bool started_;
    int numThreads_;
    int next_; //用于轮询
    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop*> loops_;
};

} // namespace muduo

#endif //__MUDUO_EVENTLOOPTHREADPOOL_H__