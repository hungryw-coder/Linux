#ifndef PRODUCERTHREAD_HPP
#define PRODUCERTHREAD_HPP

#include "../2_OO_Thread/Thread.hpp"
#include "../5_OO_TaskQueue/TaskQueue.hpp"

#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <cstdio>

using wdf::TaskQueue;
using wdf::Thread;

class ProducerThread : public Thread
{
public:
    // 构造函数 --- 初始化任务队列
    ProducerThread( TaskQueue & t )
    : m_taskQue(t)
    {

    }

private:
    // 任务队列 --- 传引用 -- 避免值传递的拷贝 和 指针传递的nullptr 对锁的影响，故使用引用
    TaskQueue & m_taskQue;

    // 重写抽象类 Thread 中的纯虚函数 run ；含有纯虚函数的类（即使只含有1个），该类也是抽象类
    void run() override
    {
        // 生产者线程主要任务是往任务队列中添加数据 
        
        int count = 10;
        while (count-- > 0) {
            int data = rand() % 100;
            m_taskQue.push(data);
            printf("Producer thread make a date: %d\n", data);
            sleep(1);   // 此时，ConsumerThread 中不 sleep ，打印结果是交替进行的
                        // 这是因为 sleep 在循环内部，每放一个元素都会休息一秒钟
                        // 这一秒钟消费者线程取数据，导致了交替打印进行
                        // 这里只是模拟，生产者消费中的 run 中的 count 不一样，可能会导致程序卡住
                        //      - 比如生产者中 count 小于消费者中的 count --- 卡在了消费者的pop中的wait
        }
    }
        
};

#endif

