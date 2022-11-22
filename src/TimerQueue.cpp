#include "TimerQueue.h"
#include "EventLoop.h"
#include "Timer.h"
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
        ts.tv_sec = static_cast<time_t>(
            microseconds / Timestamp::kMicroSecondsPerSecond);
        ts.tv_nsec = static_cast<time_t>(
            microseconds % Timestamp::kMicroSecondsPerSecond * 1000);
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

TimerId TimerQueue::addTimer(TimerCallback cb, Timestamp when, double interval)
{
    Timer* timer = new Timer(std::move(cb), when, interval);
    loop_->runInLoop(std::bind(&TimerQueue::addTimerInLoop, this, timer));
    return TimerId(timer, timer->sequence());
}

void TimerQueue::cancel(TimerId timerId)
{
    loop_->runInLoop(std::bind(&TimerQueue::cancelInLoop, this, timerId));
}

void TimerQueue::addTimerInLoop(Timer* timer)
{
    bool earliestChanged = insert(timer);
    if (earliestChanged) {
        detail::ResetTimerfd(timerfd_, timer->expiration());
    }
}

void TimerQueue::cancelInLoop(TimerId timerId)
{
    ActiveTimer timer(timerId.timer_, timerId.sequence_);
    ActiveTimerSet::iterator it = activeTimers_.find(timer);
    if (it != activeTimers_.end()) {
        timers_.erase(Entry(it->first->expiration(), it->first));
        delete it->first;
        activeTimers_.erase(it);
    } else if (callingExpiredTimers_) {
        cancelingTimers_.insert(timer);
    }
}

void TimerQueue::handleRead()
{
    Timestamp now(Timestamp::now());
    detail::ReadTimerfd(timerfd_, now);
    std::vector<Entry> expired = getExpired(now);
    callingExpiredTimers_ = true;
    cancelingTimers_.clear();
    for (const Entry& it : expired) {
        it.second->run();
    }
    callingExpiredTimers_ = false;
    reset(expired, now);
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now)
{
    std::vector<Entry> expired;
    Entry sentry(now, reinterpret_cast<Timer*>(UINTPTR_MAX));
    auto end = timers_.lower_bound(sentry);
    std::copy(timers_.begin(), end, std::back_inserter(expired));
    timers_.erase(timers_.begin(), end);
    for (const Entry& it : expired) {
        ActiveTimer timer(it.second, it.second->sequence());
        activeTimers_.erase(timer);
    }
    return expired;
}

void TimerQueue::reset(const std::vector<Entry>& expired, Timestamp now)
{
    Timestamp nextExpire;
    for (const Entry& it : expired) {
        ActiveTimer timer(it.second, it.second->sequence());
        if (it.second->repeat() && cancelingTimers_.find(timer) == cancelingTimers_.end()) {
            it.second->restart(now);
            insert(it.second);
        } else {
            delete it.second;
        }
    }
    if (!timers_.empty()) {
        nextExpire = timers_.begin()->first;
        // nextExpire = timers_.begin()->second->expiration();
    }
    if (nextExpire.valid()) {
        detail::ResetTimerfd(timerfd_, nextExpire);
    }
}

bool TimerQueue::insert(Timer* timer)
{
    bool earliestChanged = false;
    Timestamp when = timer->expiration();
    TimerList::iterator it = timers_.begin();
    if (it == timers_.end() || when < it->first) {
        earliestChanged = true;
    }
    timers_.insert(Entry(when, std::move(timer)));
    activeTimers_.insert(ActiveTimer(timer, timer->sequence()));
    return earliestChanged;
}

} // namespace muduo
