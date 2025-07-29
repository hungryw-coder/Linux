#include "Threadpool.hpp"
#include "WorkerThread.hpp"

#include <unistd.h>

namespace wdf
{

Threadpool::Threadpool(size_t threadNum, size_t queSize)
: m_threads()
, m_threadNum(threadNum)
, m_queSize(queSize)
, m_taskQue(queSize)
, isExit(false)         // 线程池退出标识位置为 false
{

}

void Threadpool::start() 
{
    // 创建N个线程对象，启动线程池
    for(size_t i = 0; i < m_threadNum; ++i) {
        // 这里需要注意：
        //      如果 WorkerThread 私有继承 Thread ，下面的代码报错
        //      因为当用unique_ptr<Thread> workthread(new WorkerThread(*this))时，需要将WorkerThread*隐式转换为Thread*
        //      由于是私有继承，这种向上转换(upcast)在类外部是被禁止的，只有WorkerThread的成员函数或友元才能执行这种转换
        //
        // 当将 WorkThread 继承改为公有继承(: public Thread)后
        //      WorkerThread现在公开宣称"是一个"Thread，允许在任何地方将WorkerThread*转换为Thread*
        //      这使得unique_ptr<Thread>能够接受new WorkerThread(...)返回的指针，因为现在允许隐式向上转换
        //      只有公开了，才能向上隐式转化
        
        // 传 this 的引用给工作线程 --- 使其能调用该类（Threadpool）中的 doTask 方法 -- 工作线程通过公有继承抽象类 Thread 重写 run 方法来执行本类的 doTask 方法执行任务
        unique_ptr<Thread> workthread(new WorkerThread(*this));  // new WorkerThread(*this) 调用的是构造函数，不是拷贝构造函数
        
        // 由于 unique_ptr 禁用拷贝构造和拷贝赋值（= delete），直接 push_back 会导致编译错误
        // m_threads.push_back(workthread);  
        
        m_threads.push_back(std::move(workthread)); // unique_ptr 不可拷贝（只能移动）
    }

    // 启动线程池 -- 让每个线程都跑起来
    // 增强 for 循环中 必须用 auto & , 因为 unique_ptr<Thread> 没有拷贝构造函数，unique_ptr 禁用拷贝构造和拷贝赋值（= delete）
    for (auto & ele : m_threads) {
        ele->start();   // 由 wdf::Thread 的内容可知，在创建线程的同时，线程入口函数会执行以动态多态性的方式执行 run 方法
    }
}


void Threadpool::stop()
{
    // 确保消费者 WorkerThread 有充足时间去取出任务并执行
    while (!m_taskQue.empty())
    {
        sleep(1);
    }
    
    isExit = true;

    m_taskQue.releaseWaitThreads();

    // 等待子线程的全部结束
    for (auto & ele : m_threads) {
        ele->join();
    }
}

void Threadpool::addTask(Task * ptask) // 充当生产者角色
{
    // 任务需要在主线程中加入 

    // 以指针的形式传递任务 --- 需要修改 TaskQueue 中 push 方法的传参类型
    if (ptask) {
        m_taskQue.push(ptask);
    }
}

void Threadpool::doTask() 
{
    // 前提: vector<unique_ptr<Thread>> m_threads 中存储的都是工作线程 WorkerThread （其中 WorkerThread 向上转型为 Thread 放入 m_threads 中）
    // doTask 的执行时机是 --- Threadpool::start --> Thread::start --> Thread::start_routine(private)  --> 动态多态 --> WorkerThread::run --> doTask
    
    // 消费者的具体实现在这里，WorkerThread 是消费者的代理，其通过多态来到这边的具体消费
    //      -  执行消费者自身的职责 -- 读取任务队列中的任务 并执行这个任务
    

    while (!isExit) {   // 线程池不关闭时，执行任务

        Task * ptask = m_taskQue.pop(); // 如果队列空了，还在消费任务，则会阻塞在 TaskQueue::pop 中
                                        // 导致这种情况有
                                        //      子线程执行速度过快，在线程池关闭时，在执行 stop 中还没有将 isExit = true 前
                                        //      子线程进入此处，此时 isExit = flase，但队列中的任务都被消费并执行了
                                        //      于是子线程阻塞在 TaskQueue::pop 方法上了
                                        //
                                        // 解决这个问题：
                                        //      需要在关闭线程池的时候，有线程被卡在了pop中的wait上，应该将他们全部唤醒
                                        //          在 TaskQueue 新增唤醒函数
        if (ptask) {
            ptask->process();           // 执行任务 -- 执行多态 -- 因为 Task 是抽象类
        }
    }

}


}
