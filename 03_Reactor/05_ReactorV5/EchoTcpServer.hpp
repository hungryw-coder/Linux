#ifndef ECHOTCPSERVER_HPP
#define ECHOTCPSERVER_HPP

#include "Threadpool.hpp"
#include "TcpServer.hpp"
#include "MyTask.hpp"

namespace wdf
{

// 是 ../04_ReactorV4/TestTcpServer.cc/ 中的进一步封装
class EchoTcpserver
{
public:
    EchoTcpserver(in_port_t port, const string & ip, size_t threadNum, size_t queSize)
    : m_threadpool(threadNum, queSize)
    , m_server(port, ip)
    {
        using namespace std::placeholders;
        m_server.setAllCallbacks(
            std::bind(&EchoTcpserver::onConnection, this, _1),
            std::bind(&EchoTcpserver::onMessage, this, _1),
            std::bind(&EchoTcpserver::onClose, this, _1));

        cout << "   EchoTcpserver(port, ip, threadNum, queSize) And set TcpServer::setAllCallbacks -- Over" << endl; 
    }
    
    void start() 
    {
        m_threadpool.start();
        m_server.start();

        cout << "   EchoTcpserver::start Over!" << endl;
    }

private:
    void onConnection(TcpConnectionPtr conn)
    {
        cout << "\n[Main] >>>> " << conn->toString() << " had connected!" << endl << endl;
    }
        
    void onMessage(TcpConnectionPtr conn) 
    {
        // onMessage 函数执行在IO线程中， 该函数对象在执行时，时间不宜过长，否则会影响并发的执行
        
        cout << ">>>> onMessage " << endl;

        // client info
        string msg = conn->receive();
        cout << "\n[Main] >>>> recv: " <<  msg << endl << endl;

        // decode
        // compute
        // encode
        
        // 往线程池中添加任务
        MyTask task(msg, conn);
        cout << ">>>> MyTask task(msg, conn)" << endl;
        m_threadpool.addTask(std::bind(&MyTask::process, task));
        // 当计算线程处理(doTask(创建线程的时候被绑定到构造函数了，各线程均卡在doTask中的pop中，start后等待任务到来时，跳出pop，执行process))完毕后，再交给IO线程进行发送
        
        cout << ">>>> m_threadpool.addTask(bind(MyTask::process)) Over" << endl;

        // 回显
        // string response = msg;
        // conn->send(response);    

    }

    void onClose(TcpConnectionPtr conn) 
    {
        cout << "\n[Main] >>>> " <<  conn->toString() << " had closed!" << endl;
    }

private:
    wdf::Threadpool m_threadpool;
    wdf::TcpServer  m_server;

};

}

#endif

