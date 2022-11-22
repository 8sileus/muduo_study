#include "poller/EpollPoller.h"
#include "poller/PollPoller.h"
#include "poller/Poller.h"

#include <stdio.h>

namespace muduo {

Poller* Poller::newDefaultPoller(EventLoop* loop)
{
    if (::getenv("MUDUO_USE_POLL")) {
        return new PollPoller(loop);
    } else {
        return new EPollPoller(loop);
    }
}

} // namespace muduo
