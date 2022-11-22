#ifndef __MUDUO_CALLBACK_H__
#define __MUDUO_CALLBACK_H__

#include "Timestamp.h"

#include <functional>
#include <memory>

namespace muduo {

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

class Buffer;
class TcpConnection;

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
using TimerCallback = std::function<void()>;
using ConnectionCallback = std::function<void(const TcpConnectionPtr&)>;
using CloseCallback = std::function<void(const TcpConnectionPtr&)>;
using WriteCompleteCallback = std::function<void(const TcpConnectionPtr&)>;
using HighWaterMarkCallback = std::function<void(const TcpConnectionPtr&, size_t)>;
using MessageCallback = std::function<void(const TcpConnectionPtr&, Buffer*, Timestamp)>;

void defaultConnectionCallback(const TcpConnectionPtr& conn);
void defaultMessageCallback(const TcpConnectionPtr& conn, Buffer* buffer, Timestamp receiveTime);

} // namespace muduo

#endif //__MUDUO_CALLBACK_H__