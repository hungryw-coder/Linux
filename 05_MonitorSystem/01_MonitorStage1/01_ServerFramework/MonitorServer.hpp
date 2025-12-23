#ifndef MONITORSERVER_HPP
#define MONITORSERVER_HPP

#include "Threadpool.hpp"
#include "TcpServer.hpp"

using wdf::TcpConnectionPtr;    // shared_ptr<void(TcpConnection)>
using wdf::Threadpool;
using wdf::TcpServer;

class MonitorServer
{
public:
    MonitorServer(size_t threadNum, size_t taskQueSize, in_port_t port, const string & ip = "0.0.0.0");

    void start();
    void stop();

private:
    void onConnection(TcpConnectionPtr conn);
    void onMessage(TcpConnectionPtr conn);
    void onClose(TcpConnectionPtr conn);

private:
    Threadpool  m_threadpool;
    TcpServer   m_server;
};


#endif

