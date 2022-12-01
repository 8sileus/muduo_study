#include "Acceptor.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "SocketAPI.h"
#include "log/Logging.h"

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

namespace muduo {

Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport)
    : loop_(loop)
    , acceptSocket_(sockets::CreateNonblockingOrDie(listenAddr.family()))
    , acceptChannel_(loop, acceptSocket_.fd())
    , listening_(false)
    , idleFd_(::open("/dev/null", O_RDONLY | O_CLOEXEC))
{
    LOG_TRACE << "Acceptor is Constructing ";
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.setReusePort(reuseport);
    acceptSocket_.bind(listenAddr);
    acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor()
{
    acceptChannel_.disableAll();
    acceptChannel_.remove();
    ::close(idleFd_);
}

void Acceptor::listen()
{
    LOG_TRACE << "Acceptor is listening ";
    listening_ = true;
    acceptSocket_.listen();
    acceptChannel_.enableReading();
}

void Acceptor::handleRead()
{
    InetAddress peerAddr;
    int connfd = acceptSocket_.accept(&peerAddr);
    InetAddress localAddr(sockets::GetLocalAddr(connfd));
    auto socket=std::make_unique<Socket>(connfd);
    if (connfd >= 0) {
        if (newConnectionCallback_) {
            newConnectionCallback_(std::move(socket),std::move(localAddr), std::move(peerAddr));
        } else {
            sockets::Close(connfd);
        }
    } else {
        LOG_SYSERR << "in Acceptor::handleRead";
        //如果文件描述符已满 则"优雅的断开"
        if (errno == EMFILE) {
            ::close(idleFd_);
            idleFd_ = ::accept(acceptSocket_.fd(), NULL, NULL);
            ::close(idleFd_);
            idleFd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
        }
    }
}

} // namespace muduo
