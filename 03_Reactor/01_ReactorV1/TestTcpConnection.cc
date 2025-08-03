#include "TcpConnection.hpp"
#include "Acceptor.hpp"
#include <iostream>

using std::cout;
using std::cerr; 
using std::endl;
using wdf::Acceptor;
using wdf::TcpConnection;

int main()
{
    cout << "main >>> Start Program -----------" << endl; 

    Acceptor acceptor(8080);
    cout << "main >>> 已创建: 服务端的监听套接字(Socket::m_fd) <- Acceptor::m_lisetnSock，以及服务端IPv4相关信息ip/port(InetAddress::m_addr) <- Acceptor::m_addr" << endl;

    acceptor.ready();
    cout << "main >>> 已完成: 服务端的setReuseAddr/port bind listen 操作" << endl;
    
    cout << "main >>> 等待客户端连接 ...  " << endl; 
    int confd = acceptor.accept();
    if (confd < 0) {
        cerr << "acceptor.accept() failed: " << strerror(errno) << endl;
        return -1;
    }
    cout << "main >>> 开始监听对端(客户端), confd = " << confd  << endl;

    sleep(1);

    TcpConnection con(confd);
    cout << "main >>> 开始与对端confd建立TCP连接 --  创建客户端套接字(Socket(int)::m_fd), 与服务端建立TCP通道(SocketIO(int)::m_fd)" << endl;
    cout << con.toString() << endl;

    string msg = con.receive();
    cout << "main >>> recv msg: " << msg <<  endl;

    con.send(msg);

    return 0;
}

