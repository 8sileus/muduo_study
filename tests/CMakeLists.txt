if(BOOSTTEST_LIBRARY)
    add_executable(Buffer_test Buffer_test.cpp)
    target_link_libraries(Buffer_test muduo_lib boost_unit_test_framework)
    add_test(NAME Buffer_test COMMAND Buffer_test)

    add_executable(InetAddress_test InetAddress_test.cpp)
    target_link_libraries(InetAddress_test muduo_lib boost_unit_test_framework)
    add_test(NAME InetAddress_test COMMAND InetAddress_test)
endif()

add_executable(AsyncLogging_test AsyncLogging_test.cpp)
target_link_libraries(AsyncLogging_test muduo_lib)

add_executable(Channel_test Channel_test.cpp)
target_link_libraries(Channel_test muduo_lib)

add_executable(EchoClient_test EchoClient_test.cpp)
target_link_libraries(EchoClient_test muduo_lib)

add_executable(EchoServer_test EchoServer_test.cpp)
target_link_libraries(EchoServer_test muduo_lib)

add_executable(EventLoop_test EventLoop_test.cpp)
target_link_libraries(EventLoop_test muduo_lib)

add_executable(EventLoopThread_test EventLoopThread_test.cpp)
target_link_libraries(EventLoopThread_test muduo_lib)

add_executable(EventLoopThreadPool_test EventLoopThreadPool_test.cpp)
target_link_libraries(EventLoopThreadPool_test muduo_lib)

add_executable(LogFile_test LogFile_test.cpp)
target_link_libraries(LogFile_test muduo_lib)

add_executable(Logging_test Logging_test.cpp)
target_link_libraries(Logging_test muduo_lib)

add_executable(LogStream_test LogStream_test.cpp)
target_link_libraries(LogStream_test muduo_lib)

add_executable(Thread_test Thread_test.cpp)
target_link_libraries(Thread_test muduo_lib)

add_executable(ThreadPool_test ThreadPool_test.cpp)
target_link_libraries(ThreadPool_test muduo_lib)

add_executable(TimerQueue_test TimerQueue_test.cpp)
target_link_libraries(TimerQueue_test muduo_lib)