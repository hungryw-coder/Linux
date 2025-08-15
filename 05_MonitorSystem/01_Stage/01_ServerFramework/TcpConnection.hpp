#ifndef TCPCONNECTION_HPP
#define TCPCONNECTION_HPP

#include "Socket.hpp"
#include "SocketIO.hpp"
#include "InetAddress.hpp"
#include "Message.hpp"

#include <memory>
#include <functional>
#include <unordered_set>

using std::shared_ptr;
using std::function;

namespace wdf
{
class EventLoop;

class TcpConnection;
using TcpConnectionPtr = shared_ptr<TcpConnection>;
using TcpConnectionCallback = function<void(TcpConnectionPtr)>;

class TcpConnection
: Noncopyable
, public std::enable_shared_from_this<TcpConnection>        // 辅助类：为了在成员函数内部函数中通过 this 字节获取本对象的智能指针 shared_ptr 
{
public:
    TcpConnection(int fd, EventLoop * loop);                // 将对端的 fd 传入 Socket(int) 与 SocketIO(int), 并保存本地地址与对端地址
                                                            // 初始化 EventLoop 的指针，为成员数据赋值
    
    ~TcpConnection() { if (m_isShutdownWrite) shutdown();  }

    // 三个回调的注册
    void setAllCallbacks(const TcpConnectionCallback & cb1,     // 参数传递时，只能使用const引用来接受临时变量，否则会导致cb指向的变量为空（这里是移动赋值运算符导致的）
                         const TcpConnectionCallback & cb2,
                         const TcpConnectionCallback & cb3)
    {
        // 不能采用移动语义，必须表达值语义 
        // --- 不能把 Eventloop 对象的三个函数对象直接转移到 TcpConnection 对象中，原因是 TcpConnection 对象不止一个
        m_onConnection = cb1;
        m_onMessage = cb2;
        m_onClose = cb3;
    }

    string receive();               // 接收对端数据
    void send(const string & msg);  // 发送数据到对端

    bool isClosed() const;          // 判断对端接收缓冲区中是否有数据，来确定对端是否关闭   
    void shutdown();                // 将对端写端关闭（不能发）, 读端依旧开启（还能收）
    string toString() const;        // 调试信息
    
    // 三个回调的执行
    // 网络编程中，针对每个连接(Servre - Client)都会发生的三件事：
    // 1、当连接建立时
    // 2、当消息到达时(client -> server)
    // 3、当连接断开时(当消息发送完成时)
    // 可以设计三个函数对象, handle 系列函数是对函数对象进行调用的
    void handleNewConnectionCallback();
    void handleMessageCallback();
    void handleCloseCallback();

    void sendInLoop(const string & msg);    // 线程池使用 TcpConnection 的对象发送数据给 EventLoop
    
    void sendInLoop(const TLV & data);      // 线程池使用 TcpConnection 的对象发送 TLV 格式数据给 EventLoop
    int readPacket(Packet & packet);        // 读取解析TLV格式后的信息的packet内容
    

    string getPeerIp() const { return m_peerAddr.ip(); }
    string getLocalIp() const { return m_localAddr.ip(); }
    in_port_t getPeerPort() const { return m_peerAddr.port(); }
    in_port_t getLocalPort() const { return m_localAddr.port(); }

    void addUser(in_port_t prot, const string & name) { m_userMap[prot] = name; } 
    string getUserName(in_port_t port) { return m_userMap[port]; } 

private:
    InetAddress getLocalAddress();  // 获取本地（服务端）地址信息 sockaddr_in 
    InetAddress getPeerAddress();   // 获取对端（客户端）地址信息 sockaddr_in

private:
    Socket      m_sock;             // 套接字对象
    SocketIO    m_sockIO;           // 套接字读写对象
    InetAddress m_localAddr;        // 本地地址
    InetAddress m_peerAddr;         // 对端地址
    bool        m_isShutdownWrite;

    TcpConnectionCallback m_onConnection;
    TcpConnectionCallback m_onMessage;
    TcpConnectionCallback m_onClose;

    EventLoop * m_loop;     // 知道 EventLoop 的存在 -- 
    
    std::unordered_map<in_port_t, string> m_userMap;   // 存放已登陆的用户
};

}

#endif


