#include "Callbacks.h"
#include "TcpConnection.h"
#include "log/Logging.h"

namespace muduo {

void DefaultConnectionCallback(const TcpConnection::Ptr& conn)
{
    LOG_TRACE << conn->localAddress().toIpPort() << " -> "
              << conn->peerAddress().toIpPort() << " is "
              << (conn->connected() ? "UP" : "DOWN");
}

void DefaultMessageCallback(const TcpConnection::Ptr&, Buffer* buf, Timestamp receiveTime)
{
    buf->retrieveAll();
}

} // namespace muduo
