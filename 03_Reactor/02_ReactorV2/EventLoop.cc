#include "Acceptor.hpp"     // 针对服务端操作
#include "EventLoop.hpp"
#include "TcpConnection.hpp"

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
{
    cout << "   EventLoop(Acceptor &) -- " << endl;
    addEpollReadEvent(m_acceptor.fd());    // 添加服务端的读事件到epoll中监听    
    cout << "   -- 获取epoll的m_epfd，初始化Acceptor、监听事件数组、m_isLooping，并将服务端读事件家加入监听" << endl;
}

EventLoop::~EventLoop()
{
    cout << "   ~EventLoop -- ";
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
            int fd = m_evtArr[i].data.fd;
            if (fd == m_acceptor.fd()) {
                // 有新连接
                handleNewConnection();
                cout << "   -- waitEpollFd handleNewConnection Over" << endl;
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
    int client_fd = m_acceptor.accept();                    // 获取新连接 -- 在这里获取对端客户端的具体信息（Socket, SocketIO）
    addEpollReadEvent(client_fd);                           // epoll实例添加对 client_fd 得监听
    TcpConnectionPtr conn(new TcpConnection(client_fd));    // 将 client_fd 传入 TcpConnection 对象中，并打通与服务端的TCP通道(SocketIO)
    conn->setAllCallbacks(m_onConnection,
                          m_onMessage,
                          m_onClose);                       // 传递三个对象给 TcpConnection 对象
    m_conns.insert(std::make_pair(client_fd, conn));        // 在 TcpConnection 容器加入新链接信息， 等价写法： m_conns[client_fd] = conn;
    conn->handleNewConnectionCallback();                    // 执行连接建立的事件
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
    cout << "EventLoop::delEpollReadEvent -- ";
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

}
