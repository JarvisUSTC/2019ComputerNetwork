
#  实现应用层上的可靠传输，实际上需要实现Go_Back_N
## 第一阶段
  已经实现数据封装，并且能够连续传输封装后的包被接收端接收
## 第二阶段
  已经实现go_back_N,timeout处理，但是测试信道收不到返回的包，如果直接运行服务器端和客户端，可以完美接收
## 第三阶段
  完美实现通过测试信道，使用了多线程并发执行send和recv
顺序运行：server_realiable_multithread.cpp testch.py client_realiable.cpp
