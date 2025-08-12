#ifndef MYTASK_HPP
#define MYTASK_HPP

#include "Task.hpp"
#include "TcpConnection.hpp"
#include <iostream>

using std::cout;
using std::endl;
using wdf::TcpConnectionPtr;

namespace wdf
{
// 不继承 Task， MyTask是个具体类
class MyTask 
{
public:
    MyTask(const string & msg, TcpConnectionPtr conn)
    : m_msg(msg)
    , m_conn(conn)
    {

    }

    void process() 
    {
        cout << "MyTask::process is running" << endl;

        // decode
        // compute
        // encode

        string response = m_msg;
        m_conn->sendInLoop(response);                       // 就是让线程池去调用TcpConnection::send 发送response 给 client 
        cout << "MyTask::process sendInLoop Over!" << endl;
        // 执行流程
        // TcpConnection::sendInLoop(msg) -> EventLoop::runInLoop(TcpConnection::send) -> EventLoop::wakeup -> EventLoop::waitFd -> handleReadEvent -> doPendingFunctors -> TcpConnection::send
        //                                                                             -> push_back(TcpConnection::send)                               
        
        // m_conn->send(response); 不能直接通过一个TCP连接来完成发信息
        // 发送信息的操作不应该在计算线程中完成，必须要通知IO线程，由IO线程最终完成数据的发送
        // 计算线程有多个，IO线程只有一个
    }

private:
    string                  m_msg;
    TcpConnectionPtr        m_conn;

};

}
#endif
