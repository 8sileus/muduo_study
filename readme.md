# muduo_study

原作者：@chenshuo。  
用 C++17实现的基于事件驱动型的高性能网络库。
为学习网络编程而写，参考《Linux 多线程服务端编程》。  

# 环境要求

- C++17
- Linux
- Cmake 3.15+

# 怎么使用？

```
./build.sh
```
or
```
sourceDir=$(pwd) &&\
mkdir -p ${sourceDir}/build && \
cd ${sourceDir}/build && \
cmake .. && \
make
```

# 一些修改

并非所有修改都为了优化程序。
部分修改使为了更好理解程序。

1. 去除 boost 依赖(部分测试文件还需要 boost)。
2. 为了更方便理解代码删除了assert(防御性编程很重要,这里只是为了学习,删除不必要的代码,使得代码更清晰)
3. 类外函数首字母统一大写。
4. 使用STL代替muduo原有的实现
   - std::mutex -> Mutex
   - std::atomic ->Atomic
   - std::condtion_variable -> Condition
   - std::promise -> CountDown
   - std::string_view -> StringPiece
   - std::any -> boost::any
5. 线程模块
   - 使用std::thread配合lambda实现。
6. 日志模块
   - 异步日志采用环形链表实现。
   - 增加日志颜色。
7. Timer模块
   - 参考sylar的timer进行重写，更方便理解。
8. TcpConnection
   - 改变了newConnection的函数形参，更直观。  
     原：std::function<void(int sockfd, const InetAddress& peerAddr)>;
     现: std::function<void(std::unique_ptr<Socket> socket,InetAddress&& localAddr, InetAddress&& peerAddr)>

# 性能测试

- 服务器配置
  - 处理器:AMD R5 3600
  - 处理器个数:2x2
  - 内存:8g
  - 线程数量:4

- 日志模块测试
  - nop: 0.195400 seconds, 109888890 bytes, 5117707.27 msg/s, 536.33 MiB/s  
  - /dev/null: 0.215226 seconds, 109888890 bytes, 4646278.80 msg/s, 486.92 MiB/s  
  - /tmp/log: 0.252342 seconds, 109888890 bytes, 3962875.78 msg/s, 415.30 MiB/s  
  - test_log_st: 0.275826 seconds, 109888890 bytes, 3625474.03 msg/s, 379.94 MiB/s  
  - test_log_mt: 0.299726 seconds, 109888890 bytes, 3336380.56 msg/s, 349.65 MiB/s    

- ab压力测试
  - 命令：./ab -n 1000000 -c 1000 -k  http://192.168.15.3:8000/hello  
    服务器：cpu占用60%~70%  
    Concurrency Level:      1000  
    Time taken for tests:   60.568 seconds  
    Complete requests:      1000000  
    Failed requests:        0  
    Keep-Alive requests:    1000000  
    Total transferred:      118000000 bytes  
    HTML transferred:       14000000 bytes  
    Requests per second:    16510.25 [#/sec] (mean)  
    Time per request:       60.568 [ms] (mean)  
    Time per request:       0.061 [ms] (mean, across all concurrent requests)  
    Transfer rate:          1902.55 [Kbytes/sec] received  

    Connection Times (ms)  
                  min  mean[+/-sd] median   max  
    Connect:        0    0   0.0      0       6  
    Processing:     1   60  35.5     53     465  
    Waiting:        0   60  35.5     53     465  
    Total:          1   60  35.5     53     465  
  
    Percentage of the requests served within a certain time (ms)  
      50%     53  
      66%     61  
      75%     66  
      80%     70  
      90%     85  
      95%    102  
      98%    228  
      99%    242  
    100%    465 (longest request)  
    
  - PingPong测试  
    客户端服务器都4线程1000连接  
    吞吐量 1019.75390625 MiB/s  
  - 对比muduo原版性能损失约2%~4%