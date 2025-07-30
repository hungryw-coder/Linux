#ifndef CONSUMERTHREAD_HPP
#define CONSUMERTHREAD_HPP

#include "../2_OO_Thread/Thread.hpp"
#include "../5_OO_TaskQueue/TaskQueue.hpp"

#include <unistd.h>
#include <cstdio>

using wdf::TaskQueue;
using wdf::Thread;

class ConsumerThread : public Thread
{
public:
    // 构造函数 --- 初始化任务队列
    ConsumerThread( TaskQueue & t)
    : m_taskQue(t)
    {
        
    }

private:
    // 任务队列 --- 传引用 -- 避免值传递的拷贝 和 指针传递的nullptr 对锁的影响，故使用引用
    TaskQueue & m_taskQue;

    // 重写抽象类 Thread 中的纯虚函数 run ；含有纯虚函数的类（即使只含有1个），该类也是抽象类
    void run() override
    {
       // 消费者的主要任务是取任务队列中的数据 
    
        int count = 10;
        while (count-- > 0) {
            int popData = m_taskQue.pop();
            printf("Consumer thread get a data: %d\n", popData);
            // sleep(1);
        }
    }
        
};

#endif

