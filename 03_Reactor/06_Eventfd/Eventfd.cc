#include "Eventfd.hpp"
#include <unistd.h>
#include <sys/eventfd.h>
#include <poll.h>
#include <string.h>
#include <iostream>

using std::cout;
using std::endl;
using std::cerr;

namespace wdf
{

Eventfd::Eventfd(EventCallback && cb)
: m_eventfd(createEventfd())
, m_cb(std::move(cb))
, m_isStarted(false)
{
            
}

Eventfd::~Eventfd()
{
    close(m_eventfd);
}

void  Eventfd::start()
{
    // 运行在子线程中
    
    // int poll(                        // 允许程序同时监视多个文件描述符,它比 select 更高效，且没有文件描述符数量的硬性限制
    //          struct pollfd *fds,     // 指定要监视的文件描述符及其关注的事件
    //          nfds_t nfds,            // 指定 fds 数组的长度（即要监视的文件描述符数量）
    //          int timeout             // 设置 poll 的超时时间, = -1：阻塞模式，无限等待直到有事件发生, = 0：非阻塞模式，立即返回（检查状态后直接退出）, > 0：等待指定的毫秒数后返回（即使无事件）
    //          ); // > 0：返回就绪的文件描述符数量（即 revents 非零的 pollfd 数量）
    //             // = 0：超时（timeout 到期且无事件发生）
    //             // = -1：出错，并设置 errno（常见错误）
    //
    // struct pollfd {
    //     int   fd;         // 要监视的文件描述符
    //     short events;     // 关心的事件（输入事件，如 POLLIN、POLLOUT）
    //     short revents;    // 实际发生的事件（输出，由内核填充）
    // };
    
    struct pollfd pfd;
    pfd.fd = m_eventfd;
    pfd.events = POLLIN;    // 数据可读（如 read 不会阻塞）
    
    m_isStarted = true;
    while(m_isStarted) {
        int nready = poll(&pfd, 1, 5000);
        if (nready == -1) {
            if (errno == EINTR) {
                continue;
            }
            cerr << "poll 失败: " << strerror(errno) << endl;
            return;
        } else if (nready == 0) {
            cout << "poll timeout." << endl;
        } else {
            handleReadEvent();
            if (m_cb) {
                m_cb();         // 执行任务
            }
        }
    }
}

void Eventfd::weakup() 
{
    uint64_t one = 1;                                       // 表示要写入的值为 1（eventfd 要求写入的数据必须是 64 位无符号整数）
    int bytes_write = write(m_eventfd, &one, sizeof(one));  // 会将该值加到 eventfd 的内部计数器上，从而唤醒监听该 eventfd 的线程/进程
    if (bytes_write != sizeof(one)) {
        cerr << "write 失败: " << strerror(errno) << endl;
    }
}

void Eventfd::stop()
{
    m_isStarted = false;
}

int Eventfd::createEventfd()
{
    // eventfd 是 Linux 系统提供的一个系统调用，用于创建一个事件通知文件描述符（eventfd），常用于进程间通信（IPC）或线程间同步
    // int eventfd(                             // 它通过一个内核维护的 64 位计数器实现事件通知机制
    //             unsigned int initval,        // 指定 eventfd 计数器的初始值, 通常设为 0，表示初始无事件
    //             int flags                    // 控制 eventfd 的行为，通过位掩码指定（可用 | 组合）: 0(阻塞模式), EFD_CLOEXEC, EFD_NONBLOCK, EFD_SEMAPHORE
    // ); // 成功：返回一个新的文件描述符（eventfd），可用于 read/write 或 poll/select
          // 失败：返回 -1，并设置 errno（如 EMFILE 表示进程文件描述符耗尽）

    int fd = eventfd(0, 0);     // 阻塞模式，初始计数器=0
    if (fd == -1) {
        cerr << "eventfd 失败: " << strerror(errno) << endl;
    }
    return fd;
}

void Eventfd::handleReadEvent()
{
    uint64_t howmany = 0;                                           // 定义一个 64 位无符号整数，用于存储读取的值
    int bytes_read = read(m_eventfd, &howmany, sizeof(howmany));    // eventfd 是一个内核维护的 64 位计数器，每次 read 会返回当前计数器的值，并根据标志位决定是否重置: 
                                                                    // 默认模式：读取后计数器 清零
                                                                    // 读取的数据必须用 uint64_t 类型存储（固定 8 字节）
    cout << "\nhowmany = " << howmany << endl;
    if (bytes_read != sizeof(howmany)) {
        cerr << "read 失败: " << strerror(errno) << endl;
    }
}

}

