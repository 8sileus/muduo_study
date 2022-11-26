# muduo_study

原作者：@chenshuo。  
用 C++实现的高质量的事件驱动型的网络库。  
参考《Linux 多线程服务端编程》
为了学习网络编程而写，性能和稳定性不做保证。

# 环境要求

- C++17
- Linux
- Cmake 3.15+

# 怎么使用？

```
./build.sh

or

sourceDir=$(pwd) &&\
mkdir -p ${sourceDir}/build && \
cd ${sourceDir}/build && \
cmake .. && \
make

```

# 一些修改(负优化)

- 为什么修改？
  - 学习 STL 的使用。
  - 方便自己理解 muduo 网络库。

1. 去除 boost 依赖(部分测试文件还需要 boost)
2. 使用了 std 标准库代替 muduo 自己的实现
   - Thread 使用 std::thread 和 lambda 表达式构成
   - std::mutex -> Mutex
   - std::atomic ->Automic
   - std::condtion_variable -> Condition
   - std::promise -> CountDown
   - std::string_view -> StringPiece
   - std::any -> boost::any
3. 非类成员函数,首字母大写

# 性能测试

- 缓冲大小:4k
- 测试时间:30s
- 处理器:AMD R5 3600
- 内存:2g
- 处理器:2x2
- 测试文件: bin/PPClient,bin/PPServer
- 单位:MiB/s
- 单线程下:  
  |连接数|1|2|3|4|  
  |---|---|---|---|---|  
  |muduo_study|77 |766 |874 |610 |  
  |muduo |69 |585 |693 |484 |

- 结论：性能上,比起原版 muduo 损失约 13%~20%的性能
- 玩具
