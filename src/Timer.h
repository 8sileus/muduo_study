// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef __MUDUO_TIMER_H__
#define __MUDUO_TIMER_H__

#include "Callbacks.h"
#include "Channel.h"
#include "Noncopyable.h"
#include "Timestamp.h"

#include <atomic>
#include <memory>
#include <set>

namespace muduo {

class EventLoop;

class Timer : Noncopyable {
public:
    using Ptr = std::shared_ptr<Timer>;

    Timer(TimerCallback cb, Timestamp when, double interval = 0)
        : callback_(std::move(cb))
        , expiration_(when)
        , interval_(interval)
        , repeat_(interval > 0.0)
    {
    }

    void run() const { callback_(); }
    Timestamp expiration() const { return expiration_; }
    bool repeat() const { return repeat_; }
    void restart(Timestamp now);

public:
    struct Compare {
        bool operator()(Timer::Ptr lhs, Timer::Ptr rhs)const;
    };

private:
    const TimerCallback callback_; //回调函数
    Timestamp expiration_; //到期时间
    const double interval_; //重复间隔
    const bool repeat_; //是否重复调用
};

class TimerQueue : Noncopyable {
public:
    explicit TimerQueue(EventLoop* loop);
    ~TimerQueue();

    std::weak_ptr<Timer> addTimer(TimerCallback cb, Timestamp when, double interval);
    void cancel(std::weak_ptr<Timer> timerId);

private:
    using TimerList = std::set<Timer::Ptr,Timer::Compare>;

    void addTimerInLoop(Timer::Ptr timer);
    void cancelInLoop(Timer::Ptr timerId);
    void handleRead();
    std::vector<Timer::Ptr> getExpired(Timestamp now);
    void reset(const std::vector<Timer::Ptr>& expired, Timestamp now);
    bool insert(Timer::Ptr timer);

private:
    EventLoop* loop_;
    const int timerfd_;
    Channel timerfdChannel_;
    // Timer list sorted by expiration
    TimerList timers_;

    // for cancel()
    TimerList activeTimers_;
    TimerList cancelingTimers_;
    std::atomic<bool> callingExpiredTimers_;
};

} // namespace muduo

#endif //__MUDUO_TIMER_H__