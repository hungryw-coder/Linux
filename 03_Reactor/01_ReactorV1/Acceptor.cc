#include "Acceptor.hpp"
// #include <sys/socket.h>
#include <iostream>

using std::cerr;
using std::endl;

namespace wdf
{

Acceptor::Acceptor(in_port_t port, const string & ip)
: m_addr(port, ip)
, m_lisetnSock()
{

}

void Acceptor::ready()
{
    setReuseAddr(true);
    setReusePort(true);
    bind();
    listen();
}

void Acceptor::setReuseAddr(bool on)
{
    // SO_REUSEADDR：允许快速重启服务器（避免 "Address already in use"）

    int one = on ? 1 : 0;
    int ret = setsockopt(m_lisetnSock.fd(), SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    if (ret < 0) {
        cerr << "setsockopt fialed: " << strerror(errno) << endl;
    }
}

void Acceptor::setReusePort(bool on)
{
    // SO_REUSEPORT：支持多进程/线程监听同一端口（Linux 3.9+）

    int one = on ? 1 : 0;
    int ret = setsockopt(m_lisetnSock.fd(), SOL_SOCKET, SO_REUSEPORT, &one, sizeof(one));
    if (ret < 0) {
        cerr << "setsockopt fialed: " << strerror(errno) << endl;
    }
}

void Acceptor::bind()
{
    int ret = ::bind(m_lisetnSock.fd(), (const struct sockaddr *)m_addr.getAddrPtr(), sizeof(m_addr));   // 显式调用系统函数--避免命名冲突
    if (ret == -1) {
        cerr << "bind fialed: " << strerror(errno) << endl;
    }
}

void Acceptor::listen()
{
    int ret =  ::listen(m_lisetnSock.fd(), 20000);              // 显式调用系统函数	避免命名冲突
    if (ret == -1) {
        cerr << "listen fialed: " << strerror(errno) << endl;
    }
}

int Acceptor::accept()
{
    // 主事件循环检测到 listenfd 可读时，调用此方法
    // 返回的 peerfd 会被封装成 TcpConnection 并注册到 epoll

    int peerfd = ::accept(m_lisetnSock.fd(), nullptr, nullptr); // 显式调用系统函数	避免命名冲突
    if (peerfd < 0) {
        cerr << "accept failed: " << strerror(errno) << endl;
    }
    return peerfd;
}

}
