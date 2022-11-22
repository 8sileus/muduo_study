#ifndef __MUDUO_TIMER_H__
#define __MUDUO_TIMER_H__

#include "Callbacks.h"
#include "Noncopyable.h"
#include "Timestamp.h"

#include <atomic>
#include <memory>

namespace muduo {

class Timer : Noncopyable {
public:
    Timer(TimerCallback cb, Timestamp when, double interval)
        : callback_(std::move(cb))
        , expiration_(when)
        , interval_(interval)
        , repeat_(interval > 0.0)
        , sequence_(s_numCreated_.fetch_add(1))
    {
    }

    void run() const
    {
        callback_();
    }

    Timestamp expiration() const { return expiration_; }
    bool repeat() const { return repeat_; }
    int64_t sequence() const { return sequence_; }

    void restart(Timestamp now);

    static int64_t numCreated() { return s_numCreated_.load(); }

private:
    static std::atomic<int64_t> s_numCreated_;

private:
    const TimerCallback callback_; //回调函数
    Timestamp expiration_; //到期时间
    const double interval_; //重复间隔
    const bool repeat_; //是否重复调用
    const int64_t sequence_; //序号:标识自己
};

/**
 * @brief Timer标识类
 */
class TimerId {
public:
    TimerId()
        : timer_(nullptr)
        , sequence_(0)
    {
    }

    TimerId(Timer* timer, int64_t seq)
        : timer_(timer)
        , sequence_(seq)
    {
    }

public:
    Timer* timer_;
    int64_t sequence_;
};

} // namespace muduo

#include <atomic>

#endif //__MUDUO_TIMER_H__