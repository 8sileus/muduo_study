add_executable(TimeServer TimeServer.cpp)
target_link_libraries(TimeServer muduo_lib)

add_executable(PPServer pingpong/server.cpp)
target_link_libraries(PPServer muduo_lib)
