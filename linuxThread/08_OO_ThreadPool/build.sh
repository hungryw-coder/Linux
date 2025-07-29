#!/bin/bash

# 编译 2_OO_Thread
g++ -std=c++11 -pthread ../2_OO_Thread/Thread.cc -c -o ../2_OO_Thread/Thread.o

# 编译 3_OO_Mutex
g++ -std=c++11 -pthread ../3_OO_Mutex/MutexLock.cc -c -o ../3_OO_Mutex/MutexLock.o

# 编译 4_OO_Condition
g++ -std=c++11 -pthread ../4_OO_Condition/Condition.cc -c -o ../4_OO_Condition/Condition.o

# 编译 5_OO_TaskQueue
g++ -std=c++11 -pthread ../5_OO_TaskQueue/TaskQueue.cc -c -o ../5_OO_TaskQueue/TaskQueue.o

# 编译 Threadpool
g++ -std=c++11 -pthread ../8_OO_ThreadPool/Threadpool.cc -c -o ../8_OO_ThreadPool/Threadpool.o

# 链接所有 .o 文件
g++ -std=c++11 -pthread \
    ../8_OO_ThreadPool/TestThreadpool.cc \
    ../2_OO_Thread/Thread.o \
    ../3_OO_Mutex/MutexLock.o \
    ../4_OO_Condition/Condition.o \
    ../5_OO_TaskQueue/TaskQueue.o \
    ../8_OO_ThreadPool/Threadpool.o \
    -o ../8_OO_ThreadPool/program

# 删除所有 .o 文件
rm -f \
    ../2_OO_Thread/Thread.o \
    ../3_OO_Mutex/MutexLock.o \
    ../4_OO_Condition/Condition.o \
    ../5_OO_TaskQueue/TaskQueue.o \
    ../8_OO_ThreadPool/Threadpool.o
