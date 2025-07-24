#include "TaskQueue.hpp"


namespace wdf
{

TaskQueue::TaskQueue(size_t queSize)
:m_queSize(queSize)
,m_metux()
,m_notFull(m_metux)
,m_notEmpty(m_metux)
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
void TaskQueue::push(int data)
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

int TaskQueue::pop()
{
    // 加锁
    // m_metux.lock();
    MutexLockGuard auto_lock(m_metux); // 离开作用域会自动释放锁
    
    // 空等
    while (empty()) {
        m_notEmpty.wait(); // R1 --- 等待队列不空
    }

    // 取对头元素 --- 将删数据
    int tem = m_que.front();

    // 删除 --- 出队
    m_que.pop();

    // 通知
    m_notFull.notify(); // R2 对应 push 忙等中的 m_notFull.wait --- 使之停止等待 --- 因为队列此时非满

    // 释放锁
    // m_metux.unlock();

    return tem;
}

}
