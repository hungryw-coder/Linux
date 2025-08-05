#include "TcpServer.hpp"
#include "TcpConnection.hpp"

using wdf::TcpConnectionPtr;
using wdf::TcpServer;

void onConnection(TcpConnectionPtr conn)
{
    cout << "\n[Main] >>>> " << conn->toString() << " had connected!" << endl << endl;
}
    
void onMessage(TcpConnectionPtr conn) 
{
    // read
    string msg = conn->receive();
    cout << "\n[Main] >>>> recv: " <<  msg << endl << endl;

    // decode
    // compute
    // encode
    
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
    cout << "\nmain >>>> Test TcpServer" << endl;
    TcpServer server(8080);
    cout << "\nmain >>>> 完成了服务端配置，以及启动了epoll事件循环" << endl;

    server.setAllCallbacks(onConnection,
                           onMessage,
                           onClose);
    cout << "\nmain >>>> 设置了三个函数对象 --> EventLoop::setAllCallbacks --> EventLoop::loop --> " 
         <<  "EventLoop::handleNewConnection --> TcpConnection::setAllCallbacks --> TcpConnection::handleNewConnectionCallback" << endl;
    
    cout << "\nmain >>>> 即将开启服务端epoll事件循环" << endl;
    server.start();

    return 0;
}

