#ifndef __MUDUO_EPOLLPOLLER_H__
#define __MUDUO_EPOLLPOLLER_H__

#include "poller/Poller.h"

#include <vector>

struct epoll_event;

namespace muduo {

class EPollPoller : public Poller {
public:
    EPollPoller(EventLoop* loop);
    ~EPollPoller() override;
    //重写基类Poller的接口
    Timestamp poll(int timeoutMs, ChannelList* activeChannels) override;
    void updateChannel(Channel* channel) override;
    void removeChannel(Channel* channel) override;

private:
    static const int kInitEventListSize = 16;

private:
    static const char* operationToString(int op);
    void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;
    void update(int operation, Channel* channel);

private:
    using EventList = std::vector<struct epoll_event>;
    int epollfd_;
    EventList events_;
};

} // namespace muduo

#endif //__MUDUO_EPOLLPOLLER_H__