// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "Timer.h"
#include "EventLoop.h"
#include "log/Logging.h"

#include <sys/timerfd.h>
#include <unistd.h>

namespace muduo {

namespace detail {

    int CreateTimerfd()
    {
        int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
        if (timerfd < 0) {
            LOG_SYSFATAL << "Failed in timerfd_create";
        }
        return timerfd;
    }

    struct timespec HowMuchTimeFromNow(Timestamp when)
    {
        int64_t microseconds = when.microSecondsSinceEpoch() - Timestamp::now().microSecondsSinceEpoch();
        if (microseconds < 100) {
            microseconds = 100;
        }
        struct timespec ts;
        ts.tv_sec = static_cast<time_t>(microseconds / Timestamp::kMicroSecondsPerSecond);
        ts.tv_nsec = static_cast<time_t>(microseconds % Timestamp::kMicroSecondsPerSecond * 1000);
        return ts;
    }

    void ReadTimerfd(int timerfd, Timestamp now)
    {
        uint64_t howmany;
        ssize_t n = ::read(timerfd, &howmany, sizeof(howmany));
        LOG_TRACE << "TimerQueue::handleRead() " << howmany << " at " << now.toFormattedString(true);
        if (n != sizeof(howmany)) {
            LOG_ERROR << "TimerQueue::handleRead() reads " << n << " bytes instead of 8";
        }
    }

    void ResetTimerfd(int timerfd, Timestamp expiration)
    {
        // wake up loop by timerfd_settime()
        struct itimerspec newValue;
        ::memset(&newValue, 0, sizeof(newValue));
        newValue.it_value = HowMuchTimeFromNow(expiration);
        int ret = ::timerfd_settime(timerfd, 0, &newValue, nullptr);
        if (ret) {
            LOG_SYSERR << "timerfd_settime()";
        }
    }
} // namespace detail

void Timer::restart(Timestamp now)
{
    if (repeat_) {
        expiration_ = addTime(now, interval_);
    } else {
        expiration_ = Timestamp::invalid();
    }
}

bool Timer::Compare::operator()(Timer::Ptr lhs, Timer::Ptr rhs) const
{
    if (lhs == nullptr && rhs == nullptr) {
        return false;
    }
    if (lhs == nullptr) {
        return false;
    }
    if (rhs == nullptr) {
        return true;
    }
    if (lhs->expiration_ < rhs->expiration_) {
        return true;
    }
    if (lhs->expiration_ > rhs->expiration_) {
        return false;
    }
    return lhs.get() < rhs.get();
}

TimerQueue::TimerQueue(EventLoop* loop)
    : loop_(loop)
    , timerfd_(detail::CreateTimerfd())
    , timerfdChannel_(loop, timerfd_)
    , timers_()
    , callingExpiredTimers_(false)
{
    timerfdChannel_.setReadCallback(std::bind(&TimerQueue::handleRead, this));
    timerfdChannel_.enableReading();
}

TimerQueue::~TimerQueue()
{
    timerfdChannel_.disableAll();
    timerfdChannel_.remove();
    ::close(timerfd_);
}

std::weak_ptr<Timer> TimerQueue::addTimer(TimerCallback cb, Timestamp when, double interval)
{
    auto timer = std::make_shared<Timer>(std::move(cb), when, interval);
    loop_->runInLoop(std::bind(&TimerQueue::addTimerInLoop, this, timer));
    return std::weak_ptr<Timer>(timer);
}

void TimerQueue::cancel(std::weak_ptr<Timer> timerId)
{
    auto ptr = timerId.lock();
    if (ptr) {
        loop_->runInLoop(std::bind(&TimerQueue::cancelInLoop, this, ptr));
    }
}

void TimerQueue::addTimerInLoop(Timer::Ptr timer)
{
    bool earliestChanged = insert(timer);
    if (earliestChanged) {
        detail::ResetTimerfd(timerfd_, timer->expiration());
    }
}

void TimerQueue::cancelInLoop(Timer::Ptr timer)
{
    auto it = activeTimers_.find(timer);
    if (it != activeTimers_.end()) {
        activeTimers_.erase(it);
    } else if (callingExpiredTimers_) {
        cancelingTimers_.insert(timer);
    }
}

void TimerQueue::handleRead()
{
    Timestamp now(Timestamp::now());
    detail::ReadTimerfd(timerfd_, now);
    std::vector<Timer::Ptr> expired = getExpired(now);
    callingExpiredTimers_ = true;
    cancelingTimers_.clear();
    for (const auto& it : expired) {
        it->run();
    }
    callingExpiredTimers_ = false;
    reset(expired, now);
}

std::vector<Timer::Ptr> TimerQueue::getExpired(Timestamp now)
{
    auto ptr = std::make_shared<Timer>(TimerCallback(), Timestamp::now());
    auto end = timers_.lower_bound(ptr);
    std::vector<Timer::Ptr> expired;
    std::copy(timers_.begin(), end, std::back_inserter(expired));
    timers_.erase(timers_.begin(), end);
    for (const auto& it : expired) {
        activeTimers_.erase(it);
    }
    return expired;
}

void TimerQueue::reset(const std::vector<Timer::Ptr>& expired, Timestamp now)
{
    Timestamp nextExpire;
    for (const auto& timer : expired) {
        if (timer->repeat() && cancelingTimers_.find(timer) == cancelingTimers_.end()) {
            timer->restart(now);
            insert(timer);
        }
    }
    if (!timers_.empty()) {
        nextExpire = (*timers_.begin())->expiration();
    }
    if (nextExpire.valid()) {
        detail::ResetTimerfd(timerfd_, nextExpire);
    }
}

bool TimerQueue::insert(Timer::Ptr timer)
{
    bool earliestChanged = false;
    Timestamp when = timer->expiration();
    auto it = timers_.begin();
    if (it == timers_.end() || when < (*it)->expiration()) {
        earliestChanged = true;
    }
    timers_.insert(timer);
    activeTimers_.insert(timer);
    return earliestChanged;
}

} // namespace muduo
