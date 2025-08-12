#include "Threadpool.hpp"

#include <unistd.h>

#include <iostream>

using std::cout;
using std::endl;

namespace wdf
{

Threadpool::Threadpool(size_t threadNum, size_t queSize)
: m_threads()                       // vector的无参构造函数没有开辟空间
, m_threadNum(threadNum)
, m_queSize(queSize)
, m_taskQue(queSize)
, isExit(false)                     // 线程池退出标识位置为 false
{
    m_threads.reserve(threadNum);   // 预留线程空间 -- 避免扩容的额外内存开销
    
    cout << "   Threadpool(threadNum, queSize) -- Over!" << endl;
}

void Threadpool::start() 
{
    // 创建N个线程对象，启动线程池
    for(size_t i = 0; i < m_threadNum; ++i) {
        
        // 将线程池的 doTask 方法注册给线程对象, 在 Thread::start --> Thread::start_routhine --> 执行 Thread::m_cb()
        unique_ptr<Thread> workthread(new Thread(std::bind(&Threadpool::doTask, this)));  // new  Thread(std::bind(&Threadpool::doTask, this)) 调用的是构造函数，不是拷贝构造函数
        
        // 由于 unique_ptr 禁用拷贝构造和拷贝赋值（= delete），直接 push_back 会导致编译错误
        // m_threads.push_back(workthread);  
        
        m_threads.push_back(std::move(workthread)); // unique_ptr 不可拷贝（只能移动）
    }

    // 启动线程池 -- 让每个线程都跑起来
    // 增强 for 循环中 必须用 auto & , 因为 unique_ptr<Thread> 没有拷贝构造函数，unique_ptr 禁用拷贝构造和拷贝赋值（= delete）
    for (auto & ele : m_threads) {
        ele->start();   // 由 wdf::Thread 的内容可知，在创建线程的同时，线程入口函数会执行 Thread::m_cb() 回调函数
    }
    
    cout << "   Threadpool::start -- 创建 " << m_threadNum  << " 个线程对象，并为每个线程绑定任务doTask，随后启动线程 Over!" << endl;
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

    cout << "   Threadpool::stop -- 已关闭线程池 Over!" << endl;
}

void Threadpool::addTask(Task && task) // 充当生产者角色
{
    // 任务需要在主线程中加入 

    // 以函数对象形式传递任务 --- 需要修改 TaskQueue 中 push 方法的传参类型
    if (task) {
        m_taskQue.push(std::move(task));
    }
    
    cout << "   Threadpool::addTask -- Over!" << endl;
}

void Threadpool::doTask() 
{
    // doTask 的执行时机是 --- Threadpool::start --> doTask绑定给Thread::m_cb  --> Thread::start 
    //                         --> Thread::start_routine(pthread->m_cb) 执行 doTask --> 从 TaskQueue 中取任务并执行(该任务是在main交给他的 MyTask::process )
    // main 将 MyTask::process 绑定成任务放入TaskQueue
    
    while (!isExit) {                   // 线程池不关闭时，执行任务

        Task task = m_taskQue.pop();    // 如果队列空了，还在消费任务，则会阻塞在 TaskQueue::pop 中
                                        // 导致这种情况有
                                        //      子线程执行速度过快，在线程池关闭时，在执行 stop 中还没有将 isExit = true 前
                                        //      子线程进入此处，此时 isExit = flase，但队列中的任务都被消费并执行了
                                        //      于是子线程阻塞在 TaskQueue::pop 方法上了
                                        //
                                        // 解决这个问题：
                                        //      需要在关闭线程池的时候，有线程被卡在了pop中的wait上，应该将他们全部唤醒
                                        //          在 TaskQueue 新增唤醒函数
        if (task) {
            task();                     // 执行任务 -- task 为函数对象 function<void()>

            cout << "   Threadpool::doTask -- popTask and excute MyTask Over" << endl;

            // 因为线程绑定的是此处的doTask函数, 一旦有任务添加进来
            // 线程从pop跳出来，执行任务
        }
    }

}


}
