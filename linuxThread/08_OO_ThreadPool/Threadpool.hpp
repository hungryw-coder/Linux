#ifndef THREADPOOL_HPP
#define THREADPOOL_HPP

#include "../2_OO_Thread/Thread.hpp"
#include "../5_OO_TaskQueue/TaskQueue.hpp"
#include "Task.hpp"

#include <memory>
#include <vector>

using std::unique_ptr;
using std::vector;

namespace wdf
{

class Threadpool
{
    friend class WorkerThread;
public:
    Threadpool( size_t, size_t );       // 构造函数 -- 初始化线程与任务队列的容量
    void start();                       // 开启线程池
    void stop();                        // 关闭线程池
    void addTask( Task * );             // 添加任务到任务队列 -- 参数是基类(抽象类)指针

private:
    void doTask();

private:
    vector<unique_ptr<Thread>>  m_threads;      // 线程容器
    size_t                      m_threadNum;    // 线程数
    size_t                      m_queSize;      // 任务队列大小
    TaskQueue                   m_taskQue;      // 任务队列
    bool                        isExit;         // 线程池是否关闭

};

}

#endif

