#include "MonitorServer.hpp"
#include "MyLogger.hpp"
#include "TcpConnection.hpp"
#include "Message.hpp"
#include "BusinessServer.hpp"

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
    
    cout << ">>> MonitorServer(threadNum, taskQueSize, port, ip) And TcpServer::setAllCallbacks -- Over!" << endl;
}

void MonitorServer::start()
{
    cout << ">>> Monitor start" << endl;
    m_threadpool.start();
    cout << ">>> threadpool started" << endl;
    m_server.start();

    cout << ">>> TcpServer started" << endl;
    cout << ">>> MonitorServer::start -- Over!" << endl;
}

void MonitorServer::stop()
{
    m_threadpool.stop();   
    m_server.stop();


    cout << ">>> MonitorServer::stop -- Over!" << endl;
}

void MonitorServer::onConnection(TcpConnectionPtr conn)
{
    wdf::MyLogger::getInstance().logDebug(conn->getLocalIp() + ":" + std::to_string(conn->getLocalPort()) 
                                          + " --> " + conn->getPeerIp() + ":" + std::to_string(conn->getPeerPort()));
    cout << ">>> MonitorServer::onConnection -- Over!" << endl;
}

void MonitorServer::onMessage(TcpConnectionPtr conn)
{ 
    // 因为这个函数通过 EventLoop 绑定到 TcpConnection 中，一但有事件（client发数据来）都会在 EventLoop::waitEpollFd 被监听到，随后处理相应的处理函数

    wdf::Packet packet; // 解析TLV格式后的信息放入packet中
    
    int ret = conn->readPacket(packet);                 // 读取客户端的packet信息
    cout << "\nread: " << ret << " bytes. \n";
    cout << "packet.type: " << packet.type << endl
         << "packet.length:" << packet.length << endl
         << "pakcet.msg:" << packet.msg << endl << endl;
    
    switch (packet.type)
    {
    case wdf::TASK_TYPE_LOGIN_SECTION1:
        {
            UserLoginSection1 userLogin1(conn, packet);                                 // 将读取客户端的packet包以及Tcp连接指针传给任务线程
            m_threadpool.addTask(std::bind(&UserLoginSection1::process, userLogin1));   // 丢给线程池来执行process函数
            cout << ">>> 向线程池添加用户注册的 section 1 的任务" << endl;
        }
        break;
    case wdf::TASK_TYPE_LOGIN_SECTION2:
        {
            UserLoginSection2 userLogin2(conn, packet);
            m_threadpool.addTask(std::bind(&UserLoginSection2::process, userLogin2));
            cout << ">>> 向线程池添加用户注册的 section 2 的任务" << endl;
        }
        break;
    }

    cout << ">>> MonitorServer::onMessage -- Over" << endl;
}

void MonitorServer::onClose(TcpConnectionPtr conn)
{
    wdf::MyLogger::getInstance().logDebug(conn->getPeerIp() + ":" + std::to_string(conn->getPeerPort()) + " had closed.");
    cout << ">>>> MonitorServer::onClose -- Over!" << endl;
}

