#ifndef __MUDUO_POLLPOLLER_H__
#define __MUDUO_POLLPOLLER_H__

#include "poller/Poller.h"

#include <poll.h>

#include <vector>

struct pollfd;

namespace muduo {

class PollPoller : public Poller {
public:
    PollPoller(EventLoop* loop)
        : Poller(loop)
    {
    }
    ~PollPoller() override = default;
    //重写基类Poller的接口
    Timestamp poll(int timeoutMs, ChannelList* activeChannels) override;
    void updateChannel(Channel* channel) override;
    void removeChannel(Channel* channel) override;

private:
    void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;

private:
    using PollFdList = std::vector<struct pollfd>;
    PollFdList pollfds_;
};

} // namespace muduo

#endif //__MUDUO_POLLPOLLER_H__