#ifndef BUSINESSSERVER_HPP
#define BUSINESSSERVER_HPP

#include "Message.hpp"
#include "TcpConnection.hpp"

class UserLoginSection1
{
public:
    UserLoginSection1(wdf::TcpConnectionPtr conn, const wdf::Packet & p)
    : m_conn(conn)
    , m_packet(p)
    {}

    //业务逻辑的处理
    void process();

private:
    void getSetting(string & s, const char * passwdf);

private:
    wdf::TcpConnectionPtr   m_conn;
    wdf::Packet             m_packet;
};

class UserLoginSection2
{
public:
    UserLoginSection2(wdf::TcpConnectionPtr conn, const wdf::Packet & p)
    : m_conn(conn)
    , m_packet(p)
    {}

    //业务逻辑的处理
    void process();


private:
    wdf::TcpConnectionPtr   m_conn;
    wdf::Packet             m_packet;
};

class UserRegisterSection1
{
public:
    UserRegisterSection1(wdf::TcpConnectionPtr conn, const wdf::Packet & p)
    : m_conn(conn), m_packet(p) {}

    void process();

private:
    wdf::TcpConnectionPtr m_conn;
    wdf::Packet m_packet;
};

class UserRegisterSection2
{
public:
    UserRegisterSection2(wdf::TcpConnectionPtr conn, const wdf::Packet & p)
    : m_conn(conn), m_packet(p) {}

    void process();

private:
    wdf::TcpConnectionPtr m_conn;
    wdf::Packet m_packet;
};

#endif

