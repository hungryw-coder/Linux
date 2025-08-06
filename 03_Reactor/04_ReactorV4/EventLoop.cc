#include "Acceptor.hpp"     // 针对服务端操作
#include "EventLoop.hpp"
#include "TcpConnection.hpp"

#include <sys/eventfd.h>
#include <iostream>

using std::cout;
using std::cerr;
using std::endl;

namespace wdf
{

EventLoop::EventLoop(Acceptor & acceptor)
: m_epfd(createEpollFd())
, m_acceptor(acceptor)
, m_isLooping(false)
, m_evtArr(1024)
, m_evtfd(createEventFd())
{
    cout << "   EventLoop(Acceptor &) -- " << endl;

    addEpollReadEvent(m_acceptor.fd());    // 添加服务端的读事件到epoll中监听    
    addEpollReadEvent(m_evtfd);            // 添加eventfd进程间通信的读事件到epoll中监听
    
    cout << "   -- EventLoop(Acceptor &) Over" << endl;
}

EventLoop::~EventLoop()
{
    cout << "   ~EventLoop -- ";
    
    close(m_evtfd);
    close(m_epfd);
    
    cout << "close(m_epfd) over" << endl;
}

void EventLoop::loop()
{
    cout << "   EventLoop::loop -- " << endl;
    cout << "   EventLoop::waitEpollFd -- " << endl;
    
    m_isLooping = true;
    while (m_isLooping) {  // 防止重复启动事件循环
        waitEpollFd();  // 开启事件循环
    }
    
    cout << "   -- loop over" << endl;
}

void EventLoop::unloop()
{
    cout << "   EventLoop::unloop -- ";
    
    m_isLooping = false;    // 要与loop函数运行在不同的线程
    
    cout << "m_isLooping = false" << endl;
}

void EventLoop::runInLoop(Functor && cb)    // cb: TcpConnection的send方法 的函数对象
{
    // 将任务(TcpConnection::send)添加到队列并唤事件循环
    cout << "   EventLoop::runInLoop -- " << endl;

    // 临界区开始
    {
        MutexLockGuard autolock(m_mutex);   // 多线程下要加锁
        m_pendings.push_back(std::move(cb));    
    } // 自动解锁
    
    wakeup();       // 通过eventfd 通知IO线程发送数据
    
    // 缺少wakeup(): 问题：如果事件循环正阻塞在epoll_wait(..., 5000ms)，新任务可能延迟最多5秒才被执行

    cout << "   -- EventLoop::runInLoop Over" << endl;
}

void EventLoop::waitEpollFd() 
{
    int nready = epoll_wait(m_epfd,             // epoll 实例的文件描述符
                            m_evtArr.data(),    // m_evtArr 是一个 vector<struct epoll_event>，动态数组，用于存储 epoll 返回的就绪事件
                                                // m_evtArr 在内存中是一段连续的数组，每个元素是一个 epoll_event 结构体, eg: vector<struct epoll_event> _evtArr(1024)
                                                // m_evtArr.data() 返回 vector 内部数组的首地址指针（struct epoll_event*）
                                                // epoll_wait 将就绪事件写入 _evtArr.data() 指向的内存, 写入的事件数量为 nready（返回值）
                            m_evtArr.size(),    // 数组大小（最多一次处理的事件数）
                            5000);              // 超时时间（毫秒），这里设置为 5 秒
    if (nready == -1) {
        if (errno == EINTR) {
            return;
        }
        cerr << "epoll_wait 失败: " << strerror(errno) << endl;
        return;
    } else if (nready == 0) {
        cout << "   epoll timeout" << endl;
    } else {
        // 有就绪事件
        cout << "   nready = " << nready << endl;
        // 处理就绪事件
        for (int i = 0; i < nready; ++i) {
            int fd = m_evtArr[i].data.fd;   // 保存着服务端监听套接字、客户端普通套接字、事件文件描述符
            if (fd == m_acceptor.fd()) {    // 当有新链接进来时，服务端的监听套接字会变为"可读"状态(可以联想到TCP三次握手成功后的全连接队列使他的状态改变)
                // 有新连接Client
                handleNewConnection();      // 在这其中将client加入epoll中监听
                cout << "   -- waitEpollFd handleNewConnection Over" << endl;
            } else if (fd == m_evtfd) {
                // 构造函数中addEpollReadEvent(m_evtfd)之后，m_evtArr 中已经存了事件文件描述符，当wakeup后，eventfd会触发读事件，使得 fd == m_evfd  TODO
                handleReadEvent();      // 处理内核计数器 -- 清零
                doPendingFunctors();    // 获取 m_pendings 中的'任务'并执行
                cout << "   -- waitEpollFd handleReadEvent and doPendingFunctors Over" << endl;
            } else {
                // 处理已连接的事件(Client)
                handleMessage(fd);
                cout << "   -- waitEpollFd handleMessage Over" << endl;
            }
        }
    }
}

void EventLoop::handleNewConnection()
{
    cout << "   EventLoop::handleNewConnection -- " << endl;
    int client_fd = m_acceptor.accept();                            // 获取新连接 -- 在这里获取对端客户端的具体信息（Socket, SocketIO）
    addEpollReadEvent(client_fd);                                   // epoll实例添加对 client_fd 得监听
    TcpConnectionPtr conn(new TcpConnection(client_fd, this));      // 将 client_fd 传入 TcpConnection 对象中，并打通与服务端的TCP通道(SocketIO), 同时传出自己的对象的指针
    conn->setAllCallbacks(m_onConnection,
                          m_onMessage,
                          m_onClose);                               // 传递三个对象给 TcpConnection 对象
    m_conns.insert(std::make_pair(client_fd, conn));                // 在 TcpConnection 容器加入新链接信息， 等价写法： m_conns[client_fd] = conn;
    conn->handleNewConnectionCallback();                            // 执行连接建立的事件
    cout << "   -- EventLoop::handleNewConnection Over" << endl;
}

void EventLoop::handleMessage(int fd) 
{
    cout << "   EventLoop::handleMessage -- " << endl;
    auto iter = m_conns.find(fd);   // 通过fd查找到对应的Tcp对象
    if (iter != m_conns.end()) {    // 找到fd对应对象
        // 判断连接是否断开
        bool isClosed = iter->second->isClosed();
        if (isClosed) {
            // 断开连接
            iter->second->handleCloseCallback();    // 调用断开时的函数对象
            delEpollReadEvent(fd);                  // 从epoll实例的监听红黑树上删除
            m_conns.erase(iter);                    // 再从map中删除
        } else {
            iter->second->handleMessageCallback();  // 有信息到来，调用消到达时的函数对象
        }
    }
    cout << "   -- EventLoop::handleMessage Over" << endl;
}

int EventLoop::createEpollFd() 
{
    cout << "   EventLoop::createEpollFd -- ";
    int epfd = epoll_create1(0);
    if (epfd < 0) {
        cerr << "epoll_create1 失败: " << strerror(errno) << endl;
    }
    cout << "epoll_create1 Over" << endl;
    return epfd;
}

void EventLoop::addEpollReadEvent(int fd) 
{
    cout << "   EventLoop::addEpollReadEvent -- ";
    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.data.fd = fd;
    ev.events = EPOLLIN;
    int ret = epoll_ctl(m_epfd, EPOLL_CTL_ADD, fd, &ev);
    if (ret == -1){
        cerr << "epoll_ctl add 失败: " << strerror(errno) << endl;
    }
    cout << "epoll_ctl-ADD Over" << endl;
}

void EventLoop::delEpollReadEvent(int fd)
{
    cout << "   EventLoop::delEpollReadEvent -- ";
    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.data.fd = fd;
    ev.events = EPOLLIN;
    int ret = epoll_ctl(m_epfd, EPOLL_CTL_DEL, fd, &ev);
    if (ret == -1){
        cerr << "epoll_ctl del 失败: " << strerror(errno) << endl;
    }
    cout << "epoll_ctl-DEL Over" << endl;
}

int EventLoop::createEventFd()
{
    cout << "   EventLoop::createEventFd -- " << endl;
    int fd = eventfd(0, 0); // 阻塞模式，他通过一个内核维护的64位计数器实现事件通知机制，初始计数器=0
    if (fd == -1) {
        cerr << "eventfd 失败: " << strerror(errno) << endl;
    }
    cout << "   -- 创建一个事件通知文件描述符，用于IPC, m_evfd = " << fd << endl;
    return fd;
}

void EventLoop::handleReadEvent()
{
    // 处理内核计数器的值(清零)
    cout << "   EventLoop::handleReadEvent -- " << endl;
    uint64_t howmany = 0;
    int bytes_read = read(m_evtfd, &howmany, sizeof(howmany));
    if (bytes_read != sizeof(howmany)) {
        cerr << "read 失败: " << strerror(errno) << endl;
    }
    cout << "   -- 将内核计数器的值清零" << endl;

}

void EventLoop::wakeup()
{
    cout << "   EventLoop::wakeup -- " << endl;
    uint64_t one = 1;
    int bytes_write = write(m_evtfd, &one, sizeof(one));
    if (bytes_write != sizeof(one)) {
        cerr << "write 失败: " << strerror(errno) << endl;
    }
    cout << "   -- 向内核计数器写值1，唤醒子线程" << endl;
}

void EventLoop::doPendingFunctors()
{
    cout << "   EventLoop::doPendingFunctors -- " << endl; 
    vector<Functor> tmp;

    {
        MutexLockGuard autolock(m_mutex); // 加锁
        tmp.swap(m_pendings);             // 交换两个 vector 的所有内容（包括底层内存指针、大小等元数据）, O(1) 复杂度的操作，不涉及元素拷贝
    }// 自动解锁
    
    // 锁只保护交换操作，不保护后续的任务执行

    // 执行任务: 执行任务时如果持有锁，阻塞其他线程
    for (auto & func : tmp) {
        func();
    }
    cout <<  "  -- EventLoop::doPendingFunctors Over!" << endl;
}

}

