#ifndef EVENTLOOP_HPP
#define EVENTLOOP_HPP

#include <map>
#include <memory>
#include <vector>
#include <functional>

using std::map;
using std::shared_ptr;
using std::vector;
using std::function;

namespace wdf
{
class TcpConnection;
using TcpConnectionPtr = shared_ptr<TcpConnection>;
using TcpConnectionCallback = function<void(TcpConnectionPtr)>;

class Acceptor;             // 向前声明，实现类中引用头文件

// 该类封装的是 epoll 的执行流程，负责事件监听和分发（epoll 管理）
class EventLoop
{
public:
    EventLoop(Acceptor &);
    ~EventLoop();
    void loop();                    // 事件循环的执行, 需要再用一个函数封装处理操作 (waitEpollFd)
    void unloop();                  // 退出事件循环
    
    // 为了给函数对象的数据成员赋值，整个 EventLoop 只是做中转，所以没有对回调执行
    void setAllCallbacks(const TcpConnectionCallback && cb1,
                         const TcpConnectionCallback && cb2,
                         const TcpConnectionCallback && cb3)
    {
        // 注册三个事件 -- 起到桥梁作用， 目的是运用移动语义将三个函数传递给 TcpConnection 对象
        // 传递时机 --- 在 handleNewConnection 中调用 TcpConnection 对象的 setAllCallbacks 函数将三个事件传递过去
        m_onConnection = std::move(cb1);
        m_onMessage = std::move(cb2);
        m_onClose = std::move(cb3);
    }
private:
    void waitEpollFd();             // 1.针对新连接的处理，设计 handleNewConnection 函数； 
                                    // 2.针对已创建好的连接(fd)进行处理，设计handleMessage函数
    void handleNewConnection();     // 该函数中会创建 TcpConnection 对象(建立与对端的 TCP 通信)
                                    // 不能使用 vector<TcpConnection> 来保存 TcpConnection 对象，原因是 TcpConnection 不能复制，但可以这么设计 vector<TcpConnection*>
                                    // 使用数组存储 TCP 连接不合适，原因是在 handleMessage 函数的调用过程中，需要通过 fd 找到 TcpConnection 对象，不适合使用 vector 存储，可以使用 map
                                    // 使用 shared_ptr 来保存 TcpConnection 对象，原因是后续在使用时需要传递 TCP 连接，不适合用 unique_ptr
                                    // 故设计 TcpConnectionPtr = shared_ptr<TcpConnection> 与 map<int, TcpConnectionPtr>
    void handleMessage(int);        // 处理已连接的 fd 进行收发信息的操作
    
    int createEpollFd();            // 创建 epoll 的文件描述符
    void addEpollReadEvent(int);    // 增加Epoll中的读事件
    void delEpollReadEvent(int);    // 删除Epoll中的读事件

private:
    int                        m_epfd;          // epoll 文件描述符，创建成功后，内核会分配一个包含红黑树与就绪链表的结构体
    Acceptor &                 m_acceptor;      // epoll 开始需要先去监听服务端的监听套接字，所以需要知道 Acceptor 的存在
    bool                       m_isLooping;     // 是否开始事件循环 -- 防止重复开启
    map<int, TcpConnectionPtr> m_conns;         // 存储该服务器对端连接的集合，存储多个 TcpConnection 对象，以及该 TcpConnection 对象对应的文件描述符
    vector<struct epoll_event> m_evtArr;        // epoll 供 epoll_wait 使用的 待监听的 TcpConnection 对象的文件描述符 数组
    
    // 当在 handleNewConnection 函数内创建 TcpConnection 之后，就要注册三个函数对象，handleNewConnection 没有传参，所以 EventLoop 内部必须设计三个函数对象，是为了转交给 TcpConnection 对象
    TcpConnectionCallback      m_onConnection;  // 新连接建立时的回调
    TcpConnectionCallback      m_onMessage;     // 收到消息时的回调
    TcpConnectionCallback      m_onClose;       // 连接关闭时的回调
};

}

#endif


