#include "Eventfd.hpp"
#include <unistd.h>
#include <ctime>
#include <iostream>
#include <thread>

using std::cout;
using std::endl;

struct MyTask
{
    void process()
    {
        cout << "make a number: " << rand() % 100 << endl;
    }
};

int main()
{
    srand(time(nullptr));
    
    wdf::Eventfd event(std::bind(&MyTask::process, MyTask()));  // 调用 createEventfd() 创建 eventfd 文件描述符, 初始化计数器为0，阻塞模式
                                                                // 存储回调函数 m_cb (这里是 MyTask::process), 设置 m_isStarted 为 false
    
    std::thread th(std::bind(&wdf::Eventfd::start, &event));    // 子线程执行 Eventfd::start() 方法, 进入 poll 循环监听 m_eventfd, 设置 m_isStarted 为 true
    
    // 子线程开始监听循环 (Eventfd::start()), 使用 poll 监听 m_eventfd 的可读事件

    for (int cnt = 10; cnt > 0; --cnt) {
        event.weakup();
        cout << "\nmain >>> thread notify cnt: " << cnt << endl;
        
        // weakup() 操作:
        // 向 m_eventfd 写入值1 (uint64_t one = 1)
        // 这会增加内核计数器的值
        // 唤醒正在 poll 等待的子线程

        // 子线程被唤醒后:
        // poll 返回大于0
        // 调用 handleReadEvent() 读取计数器值
        // 执行 MyTask::process() 打印随机数
        
        sleep(1);
    }

    sleep(1);
    event.stop();   // 设置 m_isStarted 为 false, 使子线程退出 while 循环
    th.join();      // 等待子线程结束, 子线程退出 start() 方法, gui线程继续执行直到结束

    return 0;
}
