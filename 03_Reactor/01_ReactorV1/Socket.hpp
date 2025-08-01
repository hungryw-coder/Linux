#ifndef SOCKET_HPP
#define SOCKET_HPP

#include "Noncopyable.hpp"

namespace wdf
{

// 利用 RAII 的特性管理套接字的资源 -- 构造函数中创建socket（::socket()），析构函数中自动关闭（::close()）
// 当Socket对象离开作用域时，会自动调用析构函数关闭文件描述符
class Socket
: Noncopyable               // 套接字是系统资源，一般情况下不能进行复制
{
public:
    
    Socket();               // 创建一个新的 TCP 套接字，并初始化文件描述符 m_fd      
    Socket(int);            // 使用现有的文件描述符 fd 构造 Socket 对象, 比如接受新连接（accept() 返回的 fd）
    
    ~Socket();              // 释放套接字创建的在内核对象的端点对象资源

    int fd() { return m_fd; }  // 返回 Socket 对象管理的文件描述符 m_fd
    
    void shutdownWrite();   // 调用 shutdown(fd, SHUT_WR)，关闭写方向（但仍然可以接收数据）
                            // 与close 的区别
                            //  - close() 直接关闭整个套接字
                            //  - shutdown(SHUT_WR) 只关闭写端，允许继续接收数据
    
private:
    int m_fd;               // 文件描述符 -- 通过fd()方法提供受控访问         

};

}

#endif

