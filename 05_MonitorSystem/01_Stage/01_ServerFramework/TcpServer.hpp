#ifndef TCPSERVER_HPP
#define TCPSERVER_HPP

#include "Acceptor.hpp"
#include "EventLoop.hpp"
#include "MyLogger.hpp"
#include <iostream>

using std::cout;
using std::endl;

namespace wdf 
{

// 相当于将 ../02_ReactorV2/TestEventLoop.cc 中main函数中的流程封装一遍
class TcpServer
{
public:
    TcpServer(in_port_t port, const string& ip = "0.0.0.0")
    : m_acceptor(port, ip)
    , m_loop(m_acceptor)
    {
        cout << "   TcpServer(in_port_t, const string &) -- Over!" << endl;
    }

    void setAllCallbacks(TcpConnectionCallback && cb1,
                         TcpConnectionCallback && cb2,
                         TcpConnectionCallback && cb3)
    {
        m_loop.setAllCallbacks(std::move(cb1),
                               std::move(cb2),
                               std::move(cb3));
        cout << "   TcpServer::setAllCallbacks -- Over" << endl;
    }

    void start()
    {
        cout << "   TcpServer::start -- " << endl;

        wdf::MyLogger::getInstance().logCritical("Monitor Server Start", m_acceptor.ip() + ":" + std::to_string(m_acceptor.port()));
        
        m_acceptor.ready();     // 服务端的准备工作
        m_loop.loop();          // 开始事件循环
        cout << "   -- TcpServer::start Over" << endl;
    }

    // stop 函数要与 start 函数运行在不同的线程
    void stop() 
    {
        cout << "   TcpServer::stop -- " << endl;
        m_loop.unloop();
        cout << "   -- TcpServer::stop Over" << endl;
    } 

    string ip() const { return m_acceptor.ip(); }
    in_port_t port() const { return m_acceptor.port(); }

private:
    Acceptor    m_acceptor;
    EventLoop   m_loop;
    

};

}

#endif


