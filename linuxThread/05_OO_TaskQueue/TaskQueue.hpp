#ifndef TASKQUEUE_HPP
#define TASKQUEUE_HPP

#include "../3_OO_Mutex/MutexLock.hpp"
#include "../4_OO_Condition/Condition.hpp"
#include "../8_OO_ThreadPool/Task.hpp"

#include <queue>

using std::queue;

namespace wdf
{

using ElemType = Task *;

class TaskQueue
{
public:
    // 构造函数 -- 初始化消息队列
    TaskQueue( size_t );
    // 添加元素
    void push( ElemType );
    // 删除元素
    ElemType pop();
    // 队列判空
    bool empty() const;
    // 队列判满
    bool full() const;
    // 线程池关闭时唤醒线程
    void releaseWaitThreads();

private:
    // 存储 int 型的队列
    queue<ElemType>  m_que;
    // 队列最大容量
    size_t      m_queSize;
    // 互斥锁
    MutexLock   m_metux;
    // 条件变量 --- 消息队列未满
    Condition   m_notFull;
    // 条件变量 --- 消息队列非空
    Condition   m_notEmpty;
    // 退出标识位 --- 线程池停止了
    bool flag;
};

}

#endif

