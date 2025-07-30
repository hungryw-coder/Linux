#ifndef THREADPOOL_HPP
#define THREADPOOL_HPP

#include "Task.hpp"
#include "Thread.hpp"
#include "TaskQueue.hpp"
#include "WorkThread.hpp"
#include <vector>
#include <memory>

using std::vector;
using std::unique_ptr;
using wdf::Thread;

namespace wdf
{

class Threadpool
{
    friend class WorkThread;
public:
    Threadpool(size_t threadNum, size_t queSize)
    : _threadNum(threadNum)
    , _queSize(queSize)
    , _taskQue(queSize)
    , _threads()
    , _isExit(false)
    {
        _threads.reserve(threadNum);
    }

    void start()
    {
        // 创建线程放入容器
        if (!_isExit) { //防止重复开启线程池
            for (int i = 0; i < _threadNum; ++i) {
                // Thread 抽象类无法实例化，需要一个他的派生类 --- 创建工作线程
                unique_ptr<Thread> workthread(new WorkThread(*this));
                _threads.push_back(std::move(workthread));
            }
        }
    }

    void stop()
    {

    }

    void addTask(Task * task)
    {

    }

private:
    void doTask()
    {

    }

private:
    size_t                      _threadNum;
    size_t                      _queSize;
    TaskQueue                   _taskQue;
    vector<unique_ptr<Thread>>  _threads;
    bool                        _isExit;
};


}

#endif

