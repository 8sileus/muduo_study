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
    enum Option {
        kNoReusePort,
        kReusePort,
    };

public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    TcpServer(EventLoop* loop,
        const InetAddress& listenAddr,
        const std::string& name,
        Option option = kNoReusePort);
    ~TcpServer();

    //开启监听
    void start();

    const std::string& ipPort() const { return ipPort_; }
    const std::string& name() const { return name_; }
    EventLoop* getLoop() const { return loop_; }
    void setThreadNum(int numThreads) { threadPool_->setThreadNum(numThreads); }
    std::shared_ptr<EventLoopThreadPool> threadpool() { return threadPool_; }

    //设置回调函数
    void setThreadInitCallback(const ThreadInitCallback& cb) { threadInitCallback_ = cb; }
    void setConnectionCallback(const ConnectionCallback& cb) { connectionCallback_ = cb; }
    void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }
    void setWriteCompleteCallback(const WriteCompleteCallback& cb) { writeCompleteCallback_ = cb; }

private:
    void newConnection(std::unique_ptr<Socket>socket,InetAddress&& localAddr, InetAddress&& peerAddr);
    void removeConnection(const TcpConnectionPtr& conn);
    void removeConnectionInLoop(const TcpConnectionPtr& conn);

private:
    using ConnectionMap = std::map<std::string, TcpConnection::Ptr>;

    EventLoop* loop_;
    const std::string ipPort_;
    const std::string name_;
    std::unique_ptr<Acceptor> acceptor_;
    std::shared_ptr<EventLoopThreadPool> threadPool_;
    std::atomic<int32_t> started_;
    // nextConnId_只在main loop中执行所以不需要设为原子
    int nextConnId_;
    ConnectionMap connections_;

    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    ThreadInitCallback threadInitCallback_;
};

} // namespace muduo

#endif // __MUDUO_TCPSERVER_H__