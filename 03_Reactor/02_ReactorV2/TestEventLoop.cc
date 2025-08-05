#include "EventLoop.hpp"
#include "Acceptor.hpp"
#include "TcpConnection.hpp"
#include <iostream>

using std::cout;
using std::endl;
using wdf::Acceptor;
using wdf::EventLoop;
using wdf::TcpConnectionPtr;

void onConnection(TcpConnectionPtr conn)
{
    cout << "\n[Main] >>>> " << conn->toString()  <<  " had connected! " << endl << endl;
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
    cout << "\n[Main] >>>> " <<  conn->toString() << " had closed!"<< endl;
}


int main()
{
    cout << "\nmain >>>> Start 测试EventLoop " << endl;
    // 服务端
    Acceptor acceptor(8080);      
    acceptor.ready();
    cout << "\nmain >>>> Server ready" << endl;

    // 开始epoll事件循环
    EventLoop loop(acceptor);

    cout << "\nmain >>>> 设置三个函数对象" << endl;
    loop.setAllCallbacks(onConnection,
                         onMessage,
                         onClose);
    // Q1：三个函数的调用时机？
    //
    
    cout << "\nmain >>>> 已开启epoll事件循环, 在获取新链接的时候获取服务端的 Socket 与 SocketIO" << endl;
    loop.loop();
      
    cout << "\nmain >>>> GUI going end " <<  endl;
    return 0;
}
