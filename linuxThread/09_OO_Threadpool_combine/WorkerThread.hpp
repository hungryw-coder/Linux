#ifndef WORKERTHREAD_HPP
#define WORKERTHREAD_HPP

#include "Thread.hpp"
#include "Threadpool.hpp"

namespace wdf
{

// 覆盖 Thread 的 run 方法 --- 为 Threadpool 处理 doTask(消耗任务队列里的任务)
// 存储在线程池中的工作线程容器 vector 中
// 线程池将 doTask 的任务给这个 WorkerThread 来处理（放在重写的 run 中）--- 所以要该工作线程注意到线程池的存在（将线程池作为该类的数据成员）
class WorkerThread      // 充当消费者的角色 -- 子线程（工作线程）
: public Thread         // WorkerThread 的拷贝构造函数和拷贝赋值运算符仍然会失效
                        //      Thread 私有继承 Noncopyable，导致 Thread 不可拷贝
                        //      WorkerThread 即使 公有继承 Thread，也无法绕过 Noncopyable 的限制
{
public:
    WorkerThread(Threadpool & threadpool)
    :m_threadpool(threadpool)
    {

    }

    void run() override
    {
       m_threadpool.doTask();
    }

private:
    Threadpool & m_threadpool;
};

}

#endif

