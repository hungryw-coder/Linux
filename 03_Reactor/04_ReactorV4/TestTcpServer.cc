#include "TcpServer.hpp"
#include "TcpConnection.hpp"
#include "Threadpool.hpp"
#include "MyTask.hpp"

using wdf::TcpConnectionPtr;
using wdf::TcpServer;
using wdf::Threadpool;
using wdf::MyTask;

// 创建线程池对象
Threadpool threadpool(4, 10);

void onConnection(TcpConnectionPtr conn)
{
    cout << "\n[Main] >>>> " << conn->toString() << " had connected!" << endl << endl;
}
    
void onMessage(TcpConnectionPtr conn) 
{
    // onMessage 函数执行在IO线程中， 该函数对象在执行时，时间不宜过长，否则会影响并发的执行
    
    // read
    string msg = conn->receive();
    cout << "\n[Main] >>>> recv: " <<  msg << endl << endl;

    // decode
    // compute
    // encode
    
    // 往线程池中添加任务
    MyTask task(msg, conn);
    threadpool.addTask(std::bind(&MyTask::process, task));
    // 当计算线程处理完毕后，再交给IO线程进行发送

    // 回显
    string response = msg;
    conn->send(response);    

}

void onClose(TcpConnectionPtr conn) 
{
    cout << "\n[Main] >>>> " <<  conn->toString() << " had closed!" << endl;
}

int main()
{
    cout << "\nmain >>>> Threadpoll" << endl;
    threadpool.start();

    cout << "\nmain >>>> TcpServer" << endl;
    TcpServer server(8080);
    cout << "\nmain >>>> 完成了服务端配置，以及启动了epoll事件循环" << endl;

    server.setAllCallbacks(onConnection,
                           onMessage,
                           onClose);
    cout << "\nmain >>>> 设置了三个函数对象 --> EventLoop::setAllCallbacks --> EventLoop::loop --> " 
         <<  "EventLoop::handleNewConnection --> TcpConnection::setAllCallbacks --> TcpConnection::handleNewConnectionCallback --> onConnection/onMessage" << endl;
    
    cout << "\nmain >>>> 即将开启服务端epoll事件循环" << endl;
    server.start();

    return 0;
}

