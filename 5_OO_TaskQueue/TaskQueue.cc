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

void TaskQueue::push(int data)
{
    // 加锁 -- 保证线程可以互斥的对临界区（消息队列）进行操作
    m_metux.lock(); 

    // 满等
    while (full()) {
        m_notFull.wait(); // R2 --- 等待队列不满
    }
    
    // 添加元素
    m_que.push(data);

    // 通知
    m_notEmpty.notify(); // R1: 对应 pop 忙等中的 m_notEmpty.wait --- 使之停止等待 --- 因为队列此时非空

    // 释放锁 --- 避免忙等  
    m_metux.unlock();
}

int TaskQueue::pop()
{
    // 加锁
    m_metux.lock();
    
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
    m_metux.unlock();

    return tem;
}

}
