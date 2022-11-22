#ifndef __MUDUO_TIMERQUEUE_H__
#define __MUDUO_TIMERQUEUE_H__

#include <memory>
#include <mutex>
#include <set>
#include <vector>

#include "Callbacks.h"
#include "Channel.h"
#include "Noncopyable.h"
#include "Timestamp.h"

namespace muduo {

class EventLoop;
class Timer;
class TimerId;

class TimerQueue : Noncopyable {
public:
    explicit TimerQueue(EventLoop* loop);
    ~TimerQueue();

    TimerId addTimer(TimerCallback cb, Timestamp when, double interval);
    void cancel(TimerId timerId);

private:
    using Entry = std::pair<Timestamp, Timer*>;
    using TimerList = std::set<Entry>;
    using ActiveTimer = std::pair<Timer*, int64_t>;
    using ActiveTimerSet = std::set<ActiveTimer>;

    void addTimerInLoop(Timer* timer);
    void cancelInLoop(TimerId timerId);
    void handleRead();
    std::vector<Entry> getExpired(Timestamp now);
    void reset(const std::vector<Entry>& expired, Timestamp now);
    bool insert(Timer* timer);

private:
    EventLoop* loop_;
    const int timerfd_;
    Channel timerfdChannel_;
    // Timer list sorted by expiration
    TimerList timers_;

    // for cancel()
    ActiveTimerSet activeTimers_;
    bool callingExpiredTimers_;
    ActiveTimerSet cancelingTimers_;
};

} // namespace muduo

#endif //__MUDUO_TIMERQUEUE_H__