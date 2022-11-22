#include "Thread.h"
#include "CurrentThread.h"

#include <future>

namespace muduo {

std::atomic<int32_t> Thread::numCreated_;

Thread::Thread(ThreadFunc func, const std::string& name)
    : started_(false)
    , joined_(false)
    , tid_(false)
    , func_(std::move(func))
    , name_(name)
{
    setDefaultName();
}

Thread::Thread(Thread&& other)
    : started_(other.started_)
    , joined_(other.joined_)
    , thread_(std::move(other.thread_))
    , tid_(other.tid_)
    , func_(std::move(other.func_))
    , name_(std::move(other.name_))
{
}

// Thread& Thread::operator=(Thread&& other)
// {
//     std::swap(*this, other);
//     return *this;
// }

Thread::~Thread()
{
    // if (started_ && !joined_) {
    //     thread_->detach();
    // }
}

void Thread::start()
{
    started_ = true;
    std::promise<void> p;
    thread_ = std::unique_ptr<std::thread>(new std::thread(
        [&]() {
            tid_ = current_thread::tid();
            p.set_value();
            current_thread::t_threadName = name_.c_str();
            func_();
        }));
    p.get_future().wait();
}

void Thread::join()
{
    joined_ = true;
    thread_->join();
}

void Thread::setDefaultName()
{
    int num = numCreated_.fetch_add(1);
    if (name_.empty()) {
        char buf[32];
        ::snprintf(buf, sizeof(buf), "Thread%d", num);
        name_ = buf;
    }
}

} // namespace muduo
