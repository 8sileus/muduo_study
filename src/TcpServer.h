#ifndef __MUDUO_TCPSERVER_H__
#define __MUDUO_TCPSERVER_H__

#include "EventLoopThreadPool.h"
#include "TcpConnection.h"

#include <atomic>
#include <map>

namespace muduo {

class Acceptor;
class EventLoop;
class EventLoopThreadPool;

class TcpServer : Noncopyable {
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;
    enum Option {
        kNoReusePort,
        kReusePort,
    };

    TcpServer(EventLoop* loop, const InetAddress& listenAddr, const std::string& name, Option option = kNoReusePort);
    ~TcpServer();

    //开启监听
    void start();

    const std::string& ipPort() const { return ipPort_; }
    const std::string& name() const { return name_; }
    EventLoop* getLoop() const { return loop_; }
    std::shared_ptr<EventLoopThreadPool> threadpool() { return threadPool_; }

    //设置 subloop的个数
    void setThreadNum(int numThreads) { threadPool_->setThreadNum(numThreads); }
    //设置回调函数
    void setConnectionCallback(const ConnectionCallback& cb) { connectionCallback_ = cb; }
    void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }
    void setWriteCompleteCallback(const WriteCompleteCallback& cb) { writeCompleteCallback_ = cb; }
    void setThreadInitCallback(const ThreadInitCallback& cb) { threadInitCallback_ = cb; }

private:
    void newConnection(int sockfd, const InetAddress& peerAddr);
    void removeConnection(const TcpConnectionPtr& conn);
    void removeConnectionInLoop(const TcpConnectionPtr& conn);

private:
    using ConnectionMap = std::map<std::string, TcpConnectionPtr>;

    EventLoop* loop_;
    const std::string ipPort_;
    const std::string name_;
    std::unique_ptr<Acceptor> acceptor_;
    std::shared_ptr<EventLoopThreadPool> threadPool_;
    std::atomic<int32_t> started_;
    int nextConnId_;
    ConnectionMap connections_;

    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    ThreadInitCallback threadInitCallback_;
};

} // namespace muduo

#endif // __MUDUO_TCPSERVER_H__