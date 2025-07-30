#ifndef TASKQUEUE_HPP
#define TASKQUEUE_HPP

#include "MutexLock.hpp"
#include "Condition.hpp"

#include <queue>

using std::queue;

namespace wdf
{

using ElemType = int;

class TaskQueue
{
public:
    TaskQueue(size_t queSize)
    : _que()
    , _queSize(queSize)
    , _mutex()
    , _notEmpty(_mutex)
    , _notFull(_mutex)
    {

    }

    bool empty() const 
    {
        return 0 == _que.size();
    }

    bool full() const 
    {
        return _queSize == _que.size();
    }

    void push(ElemType data)
    {
        MutexLockGuard autolock(_mutex);

        while (full()) {
            _notFull.wait();    // 等待不满
        }

        _que.push(data);
        
        _notEmpty.notifyOne();
        
    }

    ElemType pop()
    {
        ElemType tem;
        MutexLockGuard autolock(_mutex);

        while (empty()) {
            _notEmpty.wait();   // 等待不空
        }
        
        tem = _que.front();
        _que.pop();
        _notFull.notifyOne();
        
        return tem;
    }

private:
    queue<ElemType>     _que;
    size_t              _queSize;
    MutexLock           _mutex;
    Condition           _notEmpty;  // 队列不空的条件
    Condition           _notFull;   // 队列不满的条件
};

}

#endif

