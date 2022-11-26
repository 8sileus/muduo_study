# muduo_study  
用C++实现的高质量的事件驱动型的网络库, 原作者：chenshuo  
# 环境要求
- C++14
- Linux
- Cmake 3.15+
# 为什么做这个项目？  
为了学习网络编程。
参考了《Linux高性能服务器编程》和《Linux多线程服务端编程》。 
# 项目启动
sourceDir=$(pwd) &&\
mkdir -p ${sourceDir}/build && \
cd ${sourceDir}/build && \
cmake .. && \
make
# 一些修改 （负优化）
1. 去除boost依赖 //tests文件还依赖boost  
2. 使用了std标准库 替换了一些 muduo自己的实现  
	- std::thread -> Thread
	- std::mutex -> Mutex
	- std::atomic ->Automic
	- std::Conditon -> Condition	
	- std::promise -> CountDown
3. 
	
性能上,比起原版muduo损失约20%的性能
	



