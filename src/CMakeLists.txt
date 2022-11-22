# set(MUDUO_SOURCE

#     log/AsyncLogging.cpp
#     log/FileUtil.cpp
#     log/LogFile.cpp
#     log/Logging.cpp
#     log/LogStream.cpp
#     poller/DefaultPoller.cpp
#     poller/EpollPoller.cpp
#     poller/Poller.cpp
#     poller/PollPoller.cpp

#     Channel.cpp
#     CurrentThread.cpp
#     EventLoop.cpp
#     EventLoopThread.cpp
#     EventLoopThreadPool.cpp
#     InetAddress.cpp
#     SocketAPI.cpp
#     Thread.cpp
#     ThreadPool.cpp
#     Timer.cpp
#     TimerQueue.cpp
#     Timestamp.cpp
# )

# file(GLOB_RECURSE SOURCES  *.cpp *.h)

aux_source_directory(. sources)
aux_source_directory(log sources)
aux_source_directory(poller sources)

add_library(muduo_lib ${sources})
target_link_libraries(muduo_lib pthread)