#ifndef TCPCONNECTION_HPP
#define TCPCONNECTION_HPP

#include "Socket.hpp"
#include "SocketIO.hpp"
#include "InetAddress.hpp"

namespace wdf
{

class TcpConnection
{
public:
    TcpConnection(int fd);

    string receive();
    void send(const string & msg);

    bool isClosed() const;
    void shutdown();
    string toString() const;

private:
    InetAddress getLocalAddress();
    InetAddress getPeerAddress();

private:
    Socket      m_sock;             // 套接字对象
    SocketIO    m_sockIO;           // 套接字读写对象
    InetAddress m_localAddr;        // 本地地址
    InetAddress m_peerAddr;         // 对端地址

};

}

#endif

