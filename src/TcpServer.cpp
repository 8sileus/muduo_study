#include "TcpServer.h"
#include "Acceptor.h"
#include "EventLoop.h"
#include "SocketAPI.h"
#include "log/Logging.h"

#include <stdio.h>

namespace muduo {

namespace {
    static EventLoop* CheckLoopNotNull(EventLoop* loop)
    {
        if (loop == nullptr) {
            LOG_FATAL << "mainloop is nullptr!";
        }
        return loop;
    }
} // namespace

TcpServer::TcpServer(EventLoop* loop, const InetAddress& listenAddr, const std::string& name, Option option)
    : loop_(CheckLoopNotNull(loop))
    , ipPort_(listenAddr.toIpPort())
    , name_(name)
    , acceptor_(new Acceptor(loop, listenAddr, option == kReusePort))
    , threadPool_(new EventLoopThreadPool(loop, name_))
    , connectionCallback_(defaultConnectionCallback)
    , messageCallback_(defaultMessageCallback)
    , nextConnId_(1)
    , started_(0)
{
    LOG_TRACE << "TcpServer::constructing , name =  [" << name_ << "] constructing";
    // 当有有连接到达，acceptor将执行headRead()然后在里面TcpServer::newConnection
    acceptor_->setNewConnectionCallback(std::bind(&TcpServer::newConnection, this, _1, _2));
}

TcpServer::~TcpServer()
{
    LOG_TRACE << "TcpServer::~TcpServer [" << name_ << "] destructing";
    for (auto& item : connections_) {
        TcpConnectionPtr conn(item.second);
        item.second.reset();
        conn->getLoop()->runInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
    }
}

void TcpServer::start()
{
    LOG_TRACE << "TcpServer::start()";
    //防止一个TcpServer被start多次
    if (started_.fetch_add(1) == 0) {
        threadPool_->start(threadInitCallback_);
        // 开启监听 注册Acceptor的EPOLLIN事件
        loop_->runInLoop(std::bind(&Acceptor::listen, acceptor_.get()));
    }
}

void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr)
{
    // 轮询算法，保证每个loop压力一样
    EventLoop* ioLoop = threadPool_->getNextLoop();
    char buf[64];
    ::snprintf(buf, sizeof(buf), "-%s#%d", ipPort_.c_str(), nextConnId_);
    ++nextConnId_;
    std::string connName = name_ + buf;
    LOG_INFO << "TcpServer::newConnection [" << name_
             << "] - new connection [" << connName
             << "] from " << peerAddr.toIpPort();
    InetAddress localAddr(sockets::GetLocalAddr(sockfd));
    TcpConnection::Ptr conn(new TcpConnection(ioLoop, connName, sockfd, localAddr, peerAddr));
    connections_[connName] = conn;

    // 回调
    // TcpServer -> TcpConnection 以下四个
    // TcpConnection -> Channel handRead(),headWrite()...
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, _1));

    ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn)
{
    loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn)
{
    LOG_INFO << "TcpServer::removeConnectionInLoop [" << name_ << "] - connection " << conn->name();

    connections_.erase(conn->name());
    EventLoop* ioLoop = conn->getLoop();
    ioLoop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
}

} // namespace muduo
