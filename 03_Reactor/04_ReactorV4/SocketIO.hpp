#ifndef SOCKETIO_HPP
#define SOCKETIO_HPP

namespace wdf
{

// 封装面向流的 TCP 数据读写操作
class SocketIO
{
public:
    SocketIO(int fd);                               // 传入的是服务端(由 Acceptor 来创建创建服务端文件描述符)

    int sendn(const char * buff, int len);
    int recvn(char * buff, int len);

    int readline(char * buff, int maxline);         // 从套接字读取一行数据（以 \n 结尾）
    
    int recvPeek(char * buff, int max) const;       // 窥探套接字接收缓冲区的数据（不移动）

private:
    int m_fd;   // 对端文件描述符（客户端）
};

}

#endif


