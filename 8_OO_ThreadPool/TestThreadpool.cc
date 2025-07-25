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
    threadpool.start();             // 启动线程     

    // 存放任务
    unique_ptr<Task> task(new MyTask);      // new MyTask 调用构造函数，在堆上申请空间返回指针托管给智能指针来托管资源
    for (int i = 0; i < 20; ++i) {
        threadpool.addTask(task.get());     // task.get() 获取智能指针的模版类型的指针 -- Task * 
        printf("main thread has added tasks'numer is : %d\n", i + 1);
    }

    // 关闭线程池
    threadpool.stop();
    printf("main thread is ready out\n");

    return 0;
}

