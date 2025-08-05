#include "Task.hpp"
#include "Threadpool.hpp"
#include "MyTask.hpp"

using wdf::Task;
using wdf::MyTask;
using wdf::Threadpool;

int main()
{
    srand(time(nullptr));
    
    // 线程的数量根据CPU的核心数来设置 -- 通常是核数的1-2倍
    Threadpool threadpool(4, 10);   // 初始化线程池 
    threadpool.start();             // 启动线程池 -- 抽象类不能实例化，创建派生类对象向上转型成基类指针，存储进 vector
                                    //               再逐个启动线程

    // doTask 的执行时机是 --- Threadpool::start --> doTask绑定给Thread::m_cb  --> Thread::start 
    //                         --> Thread::start_routine(pthread->m_cb) 执行 doTask --> 从 TaskQueue 中取任务并执行(该任务是在main交给他的 MyTask::process )
    // main 将 MyTask::process 绑定成任务放入TaskQueue
    
    // 存放任务
    for (int i = 0; i < 20; ++i) {
        threadpool.addTask(std::bind(&MyTask::process, MyTask()));     // 将 MyTask::process 方法作为回调函数绑定成对象函数, 保存到任务队列
        printf("added tasks'numer is <<<  %d\n", i + 1);
    }

    // 关闭线程池
    threadpool.stop();
    printf("main thread is ready out\n");

    return 0;
}

