#ifndef __MUDUO_ACCEPTOR_H__
#define __MUDUO_ACCEPTOR_H__

#include "Channel.h"
#include "Noncopyable.h"
#include "Socket.h"

#include <functional>

namespace muduo {

class EventLoop;
class InetAddress;

class Acceptor : Noncopyable {
public:
    // using NewConnectionCallback = std::function<void(int sockfd, const InetAddress&)>;
    using NewConnectionCallback = std::function<void(std::unique_ptr<Socket>,InetAddress&&, InetAddress&&)>;

    Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport);
    ~Acceptor();

    void listen();

    void setNewConnectionCallback(const NewConnectionCallback& cb) { newConnectionCallback_ = cb; }
    bool listening() const { return listening_; }

private:
    void handleRead();

private:
    EventLoop* loop_;
    Socket acceptSocket_;
    Channel acceptChannel_;
    // TcpConection注册的回调函数
    NewConnectionCallback newConnectionCallback_;
    bool listening_;
    //提前占用一个文件描述符 用于在文件描述符到达上限后优雅的断开连接
    int idleFd_;
};

} // namespace muduo

#endif //__MUDUO_ACCEPTOR_H__