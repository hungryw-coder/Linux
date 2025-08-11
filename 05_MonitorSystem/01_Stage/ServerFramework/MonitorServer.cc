#include "MonitorServer.hpp"
#include "MyLogger.hpp"
#include "TcpConnection.hpp"

#include <iostream>

using std::cout;
using std::endl;

MonitorServer::MonitorServer(size_t threadNum, size_t taskQueSize, in_port_t port, const string & ip)
: m_threadpool(threadNum, taskQueSize)
, m_server(port, ip)
{
    using namespace std::placeholders;
    m_server.setAllCallbacks(
        std::bind(&MonitorServer::onConnection, this, _1),
        std::bind(&MonitorServer::onMessage, this, _1),
        std::bind(&MonitorServer::onClose, this, _1)
        );
    
    cout << "MonitorServer(threadNum, taskQueSize, port, ip) And TcpServer::setAllCallbacks -- Over!" << endl;
}

void MonitorServer::start()
{
    m_threadpool.start();
    m_server.start();

    // 记录服务器启动日志
    wdf::MyLogger::getInstance().logCriticalSystemEvent(
        "MonitorServer",
        "Server started successfully " + m_server.ip()  + " : " + std::to_string(m_server.port())
        );

    cout << "MonitorServer::start -- Over!" << endl;
}

void MonitorServer::stop()
{
    m_threadpool.stop();   
    m_server.stop();

    // 记录服务器停止日志
    wdf::MyLogger::getInstance().logCriticalSystemEvent(
        "MonitorServer",
        "Server stopped"
    );

    cout << "MonitorServer::stop -- Over!" << endl;
}

void MonitorServer::onConnection(TcpConnectionPtr conn)
{
    // 获取客户端IP和端口
    string clientIp = conn->getPeerIp();
    uint16_t clientPort = conn->getPeerPort();
    
    // 记录新连接日志
    wdf::MyLogger::getInstance().logSecurityEvent(
        "New connection", 
        "Client connected from " + clientIp + ":" + std::to_string(clientPort)
    );
}

void MonitorServer::onMessage(TcpConnectionPtr conn)
{

}

void MonitorServer::onClose(TcpConnectionPtr conn)
{
    // 获取客户端IP和端口
    std::string clientIp = conn->getPeerIp();
    uint16_t clientPort = conn->getPeerPort();
    
    // 记录连接关闭日志
    wdf::MyLogger::getInstance().logSecurityEvent(
        "Connection closed", 
        "Client disconnected from " + clientIp + ":" + std::to_string(clientPort)
    );
}

