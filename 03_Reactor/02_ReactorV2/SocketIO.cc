#include "SocketIO.hpp"
#include <my_header.h>
#include <iostream>

using std::cerr;
using std::endl;
using std::cout;

namespace wdf
{

SocketIO::SocketIO(int fd)
:m_fd(fd)
{
    cout << "   SocketIO(int) -- m_fd = " << m_fd << endl;
}

int SocketIO::sendn(const char * buff, int len)
{
    cout << "   SocketIO::sendn --";
    int remaining = len;        // 剩余需要读取的字节数
    const char * p = buff;      // 当前读取位置

    while (remaining > 0) {
        // fla=0 ：如果内核发送缓冲区满，send 会阻塞，直到数据全部写入或出错
        // 未处理 SIGPIPE：若对端关闭连接，进程可能被终止
        
        int ret = ::send(m_fd, p, remaining, 0);
        if (ret < 0) {
            cerr << "send failed: " << strerror(errno) << endl;
            return ret;
        }
        
        remaining -= ret;
        p += ret;
    }

    cout << " send " << len - remaining << "bytes, msg = " << buff << endl;
    return len - remaining;
}

int SocketIO::recvn(char * buff, int len)
{
    cout << "   SocketIO::recvn -- ";
    int remaining = len;
    char * p = buff;
    
    while (remaining > 0) {
        int ret = ::recv(m_fd, p, remaining, 0);
        if (ret == -1) {
            if (errno == EINTR) {
                continue; // 信号中断，重试
            } 
            cerr << "recv failed: " << strerror(errno) << endl;
            return ret;

        } else if (ret == 0) {
                break; // 对方已关闭连接
        }

        remaining -= ret; // 接收完全后，remaining 会变成 0
        p += ret;
    }

    cout << "recv " << len - remaining << " bytes, msg = " << buff << endl;
    return len - remaining; // 实际接收的字节数 
}


int SocketIO::readline(char * buff, int maxlen)
{   // maxlen: 缓冲区的最大容量（包括结尾的 \0）  buf: 存储读取数据的缓冲区
    // 从套接字读取一行数据（以 \n 结尾）

    cout << "   SocketIO::readline -- maxlen = "<< maxlen << endl;
    int remaining = maxlen - 1;     // 剩余可读取的字节数（保留1字节给 \0）
                                    // 可能会导致因maxlen过大，在没有找到换行符的情况下，
                                    //      remianing 没有消耗完，会一直循环（循环条件 reamaining > 0），直至读够最大长度的内容
    char * p = buff;                // 指向当前写入位置的指针
    int total = 0;                  // 表示读取的总字节数
    
    while (remaining > 0) {
        cout << "--" << endl;
        // 1. 窥探数据
        int peek_len  = recvPeek(p, remaining);         // 使用 recvPeek 窥探数据而不移除
        if (peek_len <= 0) {
            // 对端关闭(0)或出错(-1)
            if (peek_len < 0) {
                cerr << "recvPeek error: " << strerror(errno) << endl;
            }
            break; // 退出循环
        }

        // 2. 查找换行符
        for (int i = 0; i < peek_len; ++i) {
            if (p[i] == '\n') {                         // 查找到'\n'
                cout << "   >>> finded '/n' " << endl;
                int toNSize = i + 1;                    // 计算要取多少字符(包含\n) i+1 是因为 i 从 0 开始
                
                // recvn 底层调的是 recv 函数
                // 该函数的作用是在将数据从内核态读到用户态
                // 是读取而不是赋值
                // 读取完的数据，会在内核态的接收缓冲区中被删除
                int n  = recvn(p, toNSize);             // 使用 recvn 实际读取数据
                if (n != toNSize) {
                    cerr << "Error: recv failed" << endl;
                    return -1;
                }
                total += n;                             // 含\n的中总字节数
                p[i] = '\0';                            // 将\n替换成C字符床结束符\0
                cout << "   Find newline, recv " 
                     << n << " bytes " << endl;
                return total;                           // 返回已读取的总字节数（包含 \n）
            }
        }

        // 3. 未找到换行符则消费所有窥探数据
        int n = recvn(p, peek_len);                            
        if (n != peek_len){
            cerr << "[error] recv mismatch" << endl;
            return -1;
        } 
        
        p += n;
        total += n;
        remaining -= n;
    }

    // 当缓冲区将满但仍未找到 \n 时 
    buff[maxlen - 1] = '\0';                                        // 确保字符串正确终止
    cout << "   No newline found, recvn " << total << "bytes" << endl;
    return (total > 0) ? total : 0;                                 // 返回最大可读字节数
}

int SocketIO::recvPeek(char * buff, int maxlen) const
{
    // 窥探（peek）套接字接收缓冲区中的数据，但不会真正移除数据（使用 MSG_PEEK 标志）
    // 它主要用于预检查数据（例如查找 \n 换行符），而不会影响 TCP 接收窗口
    
    cout << "   SocketIO::recvPeek -- ";

    int ret = 0;
    do {
        ret = recv(m_fd, buff, maxlen, MSG_PEEK );    // 从 _fd 套接字读取最多 maxlen 字节到 buff
                                                                    // MSG_PEEK：仅窥探数据，不会从缓冲区移除（后续 recv() 仍可读取相同数据）
                                                                    // Q1:当ret=0时，程序会卡在当前recv函数中，等待内核态的接收缓冲区有数据
                                                                    //      - 修改flags 为MSG_PEEK | MSG_DONTWAIT
    } while(ret == -1 && errno == EINTR);

    if (ret < 0) {
        cerr << "recv failed: " << strerror(errno) << endl;
    } else if (ret == 0) {
        cout << "connection closed by peer" << endl;
    }

    cout << "   recvPeek: peek " << ret << " bytes" << endl;
    return ret;                                 // 返回实际窥探到的字节数（可能 < maxlen，取决于缓冲区数据量）
}

}

