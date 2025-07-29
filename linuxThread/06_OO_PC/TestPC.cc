#include "ConsumerThread.hpp"
#include "ProducerThread.hpp"


#include <memory>

using std::unique_ptr;
using wdf::Thread;
using wdf::TaskQueue;

int main()
{
    srand(time(nullptr)); // 播种
    
    // 创建任务队列 -- 容量为10 
    TaskQueue taskque(10);

    // 创建一个 Producer 与一个 Consumer 线程
    // unique_ptr<Thread> producer = new ProducerThread(taskque);
    unique_ptr<Thread> p(new ProducerThread(taskque));
    unique_ptr<Thread> c(new ConsumerThread(taskque));

    p->start();
    c->start();

    p->join();
    c->join();

    printf("ready exit!\n");
    return 0;
}

