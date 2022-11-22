#include "Channel.h"
#include "EventLoop.h"
#include "log/Logging.h"

#include <functional>
#include <map>

#include <stdio.h>
#include <sys/timerfd.h>
#include <unistd.h>

using namespace muduo;
void print(const char* msg)
{
    static std::map<const char*, Timestamp> lasts;
    Timestamp& last = lasts[msg];
    Timestamp now = Timestamp::now();
    printf("%s tid %d %s delay %f\n", now.toFormattedString(false).c_str(), current_thread::tid(),
        msg, timeDifference(now, last));
    last = now;
}

namespace muduo {
namespace detail {
    int CreateTimerfd();
    void ReadTimerfd(int timerfd, Timestamp now);
}

}

// Use relative time, immunized to wall clock changes.
class PeriodicTimer {
public:
    PeriodicTimer(EventLoop* loop, double interval, const TimerCallback& cb)
        : loop_(loop)
        , timerfd_(muduo::detail::CreateTimerfd())
        , timerfdChannel_(loop, timerfd_)
        , interval_(interval)
        , cb_(cb)
    {
        timerfdChannel_.setReadCallback(
            std::bind(&PeriodicTimer::handleRead, this));
        timerfdChannel_.enableReading();
    }

    void start()
    {
        struct itimerspec spec = { 0 };

        spec.it_interval = toTimeSpec(interval_);
        spec.it_value = spec.it_interval;
        int ret = ::timerfd_settime(timerfd_, 0 /* relative timer */, &spec, NULL);
        if (ret) {
            LOG_SYSERR << "timerfd_settime()";
        }
    }

    ~PeriodicTimer()
    {
        timerfdChannel_.disableAll();
        timerfdChannel_.remove();
        ::close(timerfd_);
    }

private:
    void handleRead()
    {
        muduo::detail::ReadTimerfd(timerfd_, Timestamp::now());
        if (cb_)
            cb_();
    }

    static struct timespec toTimeSpec(double seconds)
    {
        struct timespec ts = { 0 };
        const int64_t kNanoSecondsPerSecond = 1000000000;
        const int kMinInterval = 100000;
        int64_t nanoseconds = static_cast<int64_t>(seconds * kNanoSecondsPerSecond);
        if (nanoseconds < kMinInterval)
            nanoseconds = kMinInterval;
        ts.tv_sec = static_cast<time_t>(nanoseconds / kNanoSecondsPerSecond);
        ts.tv_nsec = static_cast<long>(nanoseconds % kNanoSecondsPerSecond);
        return ts;
    }

    EventLoop* loop_;
    const int timerfd_;
    Channel timerfdChannel_;
    const double interval_; // in seconds
    TimerCallback cb_;
};

int main(int argc, char* argv[])
{
    LOG_INFO << "pid = " << getpid() << ", tid = " << current_thread::tid()
             << " Try adjusting the wall clock, see what happens.";
    EventLoop loop;
    PeriodicTimer timer(&loop, 1, std::bind(print, "PeriodicTimer"));
    timer.start();
    loop.runEvery(1, std::bind(print, "EventLoop::runEvery"));
    loop.loop();
}
