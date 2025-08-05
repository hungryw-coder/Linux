#include "Acceptor.hpp"
// #include <sys/socket.h>
#include <iostream>

using std::cout;
using std::cerr;
using std::endl;

namespace wdf
{

Acceptor::Acceptor(in_port_t port, const string & ip)
: m_addr(port, ip)          // 生成服务器的本地地址（ip、port）
, m_lisetnSock()            // 生成服务器的文件描述符(Socket::Socket())
{
    cout << "   Acceptor(in_port_t, const string&) -- over!" << endl;
}

void Acceptor::ready()
{
    cout << "   Acceptor::ready -- " << endl;
    setReuseAddr(true);
    setReusePort(true);
    bind();
    listen();
}

void Acceptor::setReuseAddr(bool on)
{
    // SO_REUSEADDR：允许快速重启服务器（避免 "Address already in use"）

    cout << "   Acceptor::setReuseAddr -- ";
    int one = on;
    int ret = setsockopt(m_lisetnSock.fd(), SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    if (ret < 0) {
        cerr << "setsockopt fialed: " << strerror(errno) << endl;
    } else {
        cout << "setsockopt over!" << endl;
    }
}

void Acceptor::setReusePort(bool on)
{
    // SO_REUSEPORT：支持多进程/线程监听同一端口（Linux 3.9+）

    cout << "   Acceptor::setReusePort -- ";
    int one = on;
    int ret = setsockopt(m_lisetnSock.fd(), SOL_SOCKET, SO_REUSEPORT, &one, sizeof(one));
    if (ret < 0) {
        cerr << "setsockopt fialed: " << strerror(errno) << endl;
    } else {
        cout << "setsockopt over!" << endl;
    }
}

void Acceptor::bind()
{
    cout << "   Acceptor::bind -- ";
    int ret = ::bind(m_lisetnSock.fd(), (const struct sockaddr *)m_addr.getAddrPtr(), sizeof(m_addr));   // 显式调用系统函数--避免命名冲突
    if (ret == -1) {
        cerr << "bind fialed: " << strerror(errno) << endl;
    } else {
        cout << "bind over!" << endl;
    }
}

void Acceptor::listen()
{
    cout << "   Acceptor::listen -- ";
    int ret =  ::listen(m_lisetnSock.fd(), 20000);              // 显式调用系统函数	避免命名冲突
    if (ret == -1) {
        cerr << "listen fialed: " << strerror(errno) << endl;
    } else {
        cout << "listen over!" << endl;
    }
}

int Acceptor::accept()
{
    // 主事件循环检测到 listenfd 可读时，调用此方法
    // 返回的 peerfd 会被封装成 TcpConnection 并注册到 epoll

    cout << "   Acceptor::accept -- ";
    int peerfd = ::accept(m_lisetnSock.fd(), nullptr, nullptr); // ::accept()函数, 显式调用系统函数	避免命名冲突
    if (peerfd < 0) {
        cerr << "accept failed: " << strerror(errno) << endl;
    } else {
        cout << "accept peerfd = " << peerfd << endl;
    }

    return peerfd;
}

}
