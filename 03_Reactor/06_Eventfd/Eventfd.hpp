#ifndef EVENTFD_HPP
#define EVENTFD_HPP

#include <functional>

using std::function;

namespace wdf
{

using EventCallback = function<void()>;

// 线程 A 调用 wakeup() 写入 1，唤醒正在 read(_eventfd) 阻塞的线程 B
class Eventfd
{
public:
    Eventfd(EventCallback && cb);
    ~Eventfd();
    
    void start();               // start 方法内部不断对 m_eventfd 进行监听，监听使用 IO多路复用的 poll
                                // 如股有另一个线程通知 m_eventfd ，就去执行回调函数 m_cb
                                // 调用start方法的是一个子线程
    
    void weakup();              // 另一个线程（主线程）要通知 m_eventfd 时，就调用 write 函数修改内核计数器的值，该操作由此函数来完成
    void stop();                // 当子线程不再监听 m_eventfd 时，调用该函数停止监听
    
private:
    int createEventfd();        // 用于给成员数赋值使用，生成poll实例和文件描述符
    void handleReadEvent();     // 当 m_eventfd 对应的内核计数器的值被修改，就会触发读事件，调用该函数进行处理

private:
    int             m_eventfd;
    EventCallback   m_cb;
    bool            m_isStarted;

};

}

#endif

