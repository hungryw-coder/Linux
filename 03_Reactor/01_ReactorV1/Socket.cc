#include "Socket.hpp"
#include <my_header.h>
#include <iostream>

using std::cout;
using std::endl;
using std::cerr;

namespace wdf 
{

// int shutdown(                // shutdown() 是一个用于 控制套接字（socket）关闭行为 的系统调用 
//              int sockfd,     // 要操作的套接字文件描述符
//              int how         // 关闭方式
//              );
// SHUT_RD (0)	    关闭读端	    不再接收数据（但可以继续发送）
// SHUT_WR (1)	    关闭写端    	不再发送数据（但可以继续接收）
// SHUT_RDWR (2)	关闭读写两端	相当于 SHUT_RD + SHUT_WR


Socket::Socket()
{
    m_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_fd < 0) {
        cerr << "Socket init failed: " << strerror(errno) << endl;
    }
}

Socket::Socket(int fd) 
: m_fd(fd)
{

}

Socket::~Socket()
{
    close(m_fd);
    cout << "Socket " << m_fd << "has closed." << endl;
}


void Socket::shutdownWrite()
{
    shutdown(m_fd, SHUT_WR);
}

}
