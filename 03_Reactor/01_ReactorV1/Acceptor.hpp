#ifndef ACCEPTOR_HPP
#define ACCEPTOR_HPP

#include "Socket.hpp"
#include "InetAddress.hpp"

namespace wdf
{

// Acceptor 时一个新链接的接收器， 监听的服务器
class Acceptor
{
public:
    Acceptor(in_port_t port, const string & ip = "0.0.0.0");    // 初始化 InetAddress（绑定 IP 和端口）和 Socket（创建监听套接字）
    void ready();                                               // 一键完成监听套接字的初始化 -- 设置 SO_REUSEADDR/SO_REUSEPORT（避免 TIME_WAIT 问题）；绑定地址（bind()）；开始监听（listen()）
    int accept();                                               // 返回新连接的 fd，方便 Reactor 将其注册到 epoll 事件循环中

private:
    void setReuseAddr(bool);            // 设置 SO_REUSEADDR 优化服务器重启时的端口占用问题
    void setReusePort(bool);            // 设置 SO_REUSEPORT 支持多进程/线程监听同一端口
    void bind();                        // 绑定地址
    void listen();                      // 开始监听 

private:
    InetAddress     m_addr;             // 封装服务器的 IP 地址和端口（如 "0.0.0.0:8080"），提供 sockaddr 格式的地址信息
    Socket          m_lisetnSock;       // 由于 Socket 继承自 Noncopyable，导致 Acceptor 也隐式成为不可拷贝的类
                                        // Q1：为什么不应该允许拷贝？
                                        // A1：核心问题：资源所有权
                                        //      1. 文件描述符重复关闭：如果允许拷贝，多个 Acceptor 对象会持有同一个监听socket的fd，析构时会导致重复关闭
                                        //      2. 监听端口冲突：拷贝后的多个 Acceptor 会尝试监听相同端口
                                        //     网络编程最佳实践
                                        //      1. 监听套接字应该具有唯一所有权
                                        //      2. 符合RAII原则，避免资源管理混乱


};

}

#endif

