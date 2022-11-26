#include "Endian.h"
#include "EventLoop.h"
#include "TcpServer.h"
#include "Timestamp.h"
#include "log/Logging.h"

#include <unistd.h>

using namespace muduo;

class TimeServer {
public:
    TimeServer(EventLoop* loop, const InetAddress& listenaddr)
        : server_(loop, listenaddr, "TimeServer")
    {
        server_.setConnectionCallback(
            [&](const TcpConnectionPtr& conn) {
                LOG_INFO << "TimeServer - " << conn->peerAddress().toIpPort() << " -> "
                         << conn->localAddress().toIpPort() << " is "
                         << (conn->connected() ? "UP" : "DOWN");
                if (conn->connected()) {
                    auto now = Timestamp::now();
                    conn->send(now.toFormattedString(false));
                    conn->shutdown();
                }
            });
        server_.setMessageCallback(
            [&](const TcpConnectionPtr& conn, Buffer* buf, Timestamp time) {
                std::string msg(buf->retrieveAllAsString());
                LOG_INFO << conn->name() << " discards " << msg.size()
                         << " bytes received at " << time.toFormattedString(false);
            });
    }

    void start()
    {
        server_.start();
    }

private:
    TcpServer server_;
};

int main(int argc, char** argv)
{
    // Logger::setLogLevel(Logger::TRACE);

    InetAddress listenAddr;
    if (argc == 2) {
        listenAddr = InetAddress(atoi(argv[1]));
    } else if (argc == 3) {
        listenAddr = InetAddress(argv[1], atoi(argv[2]));
    } else {
        printf("please usage: ./TimeServer [ip] port \n");
        return -1;
    }

    LOG_INFO << "pid = " << getpid();
    printf("%s\n", listenAddr.toIpPort().c_str());
    EventLoop loop;
    TimeServer server(&loop, listenAddr);
    server.start();
    loop.loop();
    return 0;
}