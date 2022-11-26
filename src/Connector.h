#ifndef __MUDUO_CONNECTOR_H__
#define __MUDUO_CONNECTOR_H__

#include "InetAddress.h"
#include "Noncopyable.h"

#include <functional>
#include <memory>

namespace muduo {

class Channel;
class EventLoop;

class Connector : Noncopyable,
                  public std::enable_shared_from_this<Connector> {
public:
    using Ptr = std::shared_ptr<Connector>;
    using NewConnectionCallback = std::function<void(int)>;

    Connector(EventLoop* loop, const InetAddress& serverAddr);
    ~Connector();

    void start();
    void restart();
    void stop();

    const InetAddress& serverAddress() const { return serverAddr_; }
    void setNewConnectionCallback(const NewConnectionCallback& cb) { newConnectionCallback_ = cb; }

private:
    static const int kMaxRetryDelayMs = 30 * 1000;
    static const int kInitRetryDelayMs = 500;

private:
    enum States {
        kDisconnected,
        kConnecting,
        kConnected
    };

    void setState(States s) { state_ = s; }
    void startInLoop();
    void stopInLoop();
    void connect();
    void connecting(int sockfd);
    void handleWrite();
    void handleError();
    void retry(int sockfd);
    int removeAndResetChannel();
    void resetChannel();

private:
    EventLoop* loop_;
    InetAddress serverAddr_;
    bool connect_;
    States state_;
    std::unique_ptr<Channel> channel_;
    NewConnectionCallback newConnectionCallback_;
    int retryDelayMs_;
};

} // namespace mudou

#endif //__MUDUO_CONNECTOR_H__