# muduo_study  
原作者：@chenshuo。  
用C++实现的高质量的事件驱动型的网络库。  
项目为了学习网络编程而写，性能和稳定性不做保证。  
# 环境要求
- C++14
- Linux
- Cmake 3.15+
# 为什么做这个项目？  
为了学习网络编程。
参考了《Linux高性能服务器编程》和《Linux多线程服务端编程》。 
# 项目启动
```
sourceDir=$(pwd) &&\
mkdir -p ${sourceDir}/build && \
cd ${sourceDir}/build && \
cmake .. && \
make
```
# 一些修改 （负优化）
- 为什么修改？  
	- 练习STL库的使用。
	- 把一些具体的实现忽略，方便自己理解muduo网络库。  
1. 去除boost依赖 //tests文件还依赖boost  
2. 使用了std标准库代替muduo自己的实现  
	- std::thread -> Thread
	- std::mutex -> Mutex
	- std::atomic ->Automic
	- std::condtion_variable -> Condition	
	- std::promise -> CountDown
3. 部分的函数使用了lambda代替bind
4. 整合了部分文件。  
5. 对函数名进行了一些调整(非类成员函数首字母大写)  
# 性能测试
- 缓冲大小:4k
- 测试时间:30s  
- 处理器:AMD R5 3600  
- 内存:2g  
- 处理器:2x2  
- 测试文件: bin/PPClient bin/PPServer

- 单线程下:  单位:MiB/s  
	|连接数|1|2|3|4|  
	|---|---|---|---|---|    
	|muduo_study|77  |766 |874 |610 |   
	|muduo 	    |69	 |585 |693 |484 |  
	

- 结论：性能上,比起原版muduo损失约13%~20%的性能 玩具
	



