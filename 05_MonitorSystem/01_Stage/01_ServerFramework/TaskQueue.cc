#include "TaskQueue.hpp"


namespace wdf
{

TaskQueue::TaskQueue(size_t queSize)
:m_queSize(queSize)
,m_metux()
,m_notFull(m_metux)
,m_notEmpty(m_metux)
,flag(true)
{

}

bool TaskQueue::empty() const
{
    return 0 == m_que.size(); 
}

bool TaskQueue::full() const
{
    return m_queSize == m_que.size();
}

// 想要实现上锁后自动解锁 --- RAII技术 --- 在 MutexLock 的基础上再封装一个类，在该类中的析构函数中加入释放锁操作 
// 此时可以实现类似 c++ 中 lock_guard 功能的函数
void TaskQueue::push(ElemType data)
{
    // 加锁 -- 保证线程可以互斥的对临界区（消息队列）进行操作
    // m_metux.lock(); 
    MutexLockGuard auto_lock(m_metux); // 离开作用域会自动释放锁

    // 满等
    while (full()) {
        m_notFull.wait(); // R2 --- 等待队列不满
    }
    
    // 添加元素
    m_que.push(data);

    // 通知
    m_notEmpty.notify(); // R1: 对应 pop 忙等中的 m_notEmpty.wait --- 使之停止等待 --- 因为队列此时非空

    // 释放锁 --- 避免忙等  
    // m_metux.unlock(); // 引入 MutexLockGuard 后无需手动释放锁
}

ElemType TaskQueue::pop()
{
    // 加锁
    // m_metux.lock();
    MutexLockGuard auto_lock(m_metux); // 离开作用域会自动释放锁
    
    ElemType tem;

    // 空等
    while (flag && empty()) {   // 1. 当线程池一直开启时，任务队列为空时，子线程便会进来，卡在wait等待任务进来再消费
                                //
                                // 2. 当线程池关闭时，任务队列为空的话，子线程进来，卡在wait
                                //    随着程序执行到 Threadpool::stop 唤醒这里等待的进程，正确退出
                                //
                                // 3. 会不会存在着任务队列里的任务还没被消费就线程池退出的问题？
                                //      - 为了避免这种情况，在 Threadpool 中，sleep 一下，确保有充足时间去消费任务（并执行任务 -- Task 的多态实现）

        m_notEmpty.wait();      // R1 --- 等待队列不空
                                //
                                // 关闭线程池时，因线程并发阻塞到此处的线程，在 releaseWaitThreads 操作后被唤醒
                                // 但是会产生新的问题 --- 任务队列是空的，while 循环依旧继续，程序有卡在wait处
                                //
                                // 解决方法：
                                //   添加一个退出标识位来确认线程池关闭了
    }
    
    // 为了避免线程并发带来的影响，要确保在线程池不退出的情况下删除数据（任务）
    if (flag) {

        // 取对头元素 --- 将删数据
        tem = m_que.front();
        // 删除 --- 出队
        m_que.pop();
        // 通知
        m_notFull.notify(); // R2 对应 push 忙等中的 m_notFull.wait --- 使之停止等待 --- 因为队列此时非满

    } else {

        tem = nullptr;
    }


    // 释放锁
    // m_metux.unlock();

    return tem;
}

void TaskQueue::releaseWaitThreads()
{
    // 在线程池关闭时，唤醒所有因并发导致的线程卡在pop中的线程, 在此之前确保 pop 中的while循环能正确退出，还需要添加一个退出标识位的成员数据
    
    flag = false;           // 线程池在退出, 正确退出 pop 中的 while 循环
    m_notEmpty.notifyAll();
}

}
