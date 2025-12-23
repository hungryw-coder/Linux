#include "TcpConnection.hpp"
#include "EventLoop.hpp"
#include <iostream>
#include <sstream>

using std::cout;
using std::cerr;
using std::endl;


namespace wdf
{

TcpConnection::TcpConnection(int fd, EventLoop * loop)
: m_sock(fd)                            // 将对端fd传入, 构建套接字对象, 服务端套接字在Acceptor中进行创建
, m_sockIO(fd)                          // 将对端fd传入, 进行TCP通信
, m_localAddr(getLocalAddress())        // 调用的是 InetAddress::InetAddress(const struct sockaddr_in &) 保存了本地地址协议
, m_peerAddr(getPeerAddress())          // 同理, 保存了对端协议地址, 由 m_peerAddr 保存，m_peerAddr -> InetAddress <- getPeerAddress()
, m_isShutdownWrite(false)    
, m_loop(loop)                          // 将 EventLoop 指针传进来 -- 通过 EventLoop::handleNewConnection 
{
    cout << "   TcpConnection(int, loop) -- over!" << endl;
}

string TcpConnection::receive()
{
    cout << "   TcpConnection::receive -- " << endl;
    char buff[1024] = {0};                             // 64 KB 栈缓冲区
    int bytes_readline = m_sockIO.readline(buff, sizeof(buff));
    cout << "   -- TcpConnection::receive readline: " << bytes_readline << "bytes " << endl;
    return string(buff, bytes_readline);
}

void TcpConnection::send(const string & msg)            // const string& 避免拷贝
{
    cout << "   TcpConnection::send -- " << endl;
    if (msg.length() > 0) {                             // 调用 SocketIO::sendn 发送全部数据
        m_sockIO.sendn(msg.c_str(), msg.length());      // string 类型的size()与lengthlength()函数是完全一样的，返回的是字符串的个数（不含\0）
    } 
    cout << "   -- TcpConnection::send sendn over!" << endl;
}

bool TcpConnection::isClosed() const 
{
    cout << "   TcpConnection::isClosed -- SocketIO::recvPeek == 0 ?" << endl;
    char buf[20] = {0};
    return m_sockIO.recvPeek(buf, sizeof(buf)) == 0;    // 通过 recvPeek 窥探对端数据（不移动读取指针）, 返回0则表示对端已关闭连接
}

void TcpConnection::shutdown()
{
    cout << "   TcpConnection::shutdown -- Socket::shutdownWrite" << endl;
    m_sock.shutdownWrite();                             // 调用 Socket::shutdownWrite 关闭写端（发送FIN包）,但保留读端 -- 半关闭
}

string TcpConnection::toString() const
{                                                       // 调试信息
    cout << "   TcpConnection::toString -- " << endl;
    std::ostringstream oss;                             // 输出字符串流，允许像使用 std::cout 一样通过 << 操作符将多种类型的数据（如字符串、数字、地址等）拼接成一个字符串
    oss << "[TCP] " << m_localAddr.ip() << ":" << m_localAddr.port()
        << " -> " 
        << m_peerAddr.ip() << ":" << m_peerAddr.port();
    return oss.str();                                   // 将流内容转换为 std::string 并返回
}

void TcpConnection::handleNewConnectionCallback()
{
    cout << "   TcpConnection::handleNewConnectionCallback -- " << endl;
    if (m_onConnection) {                       // 不为空，在 EventLoop::handleNewConnection 中已通过 setAllCallbacks 将他的成员数数据（回调函数）传到该类中，所以非空
        m_onConnection(shared_from_this());     // 在成员函数内部正确获取本对象的 shared_ptr
                                                // TcpConnection 继承 std::enable_shared_from_this, 在使用shared_from_this传入本类对象的智能指针
        
        // 在main函数将 onConnection 函数通过 EventLoop 中 setAllCallbacks 的移动语义赋值给 TcpConnection 对象, 
        // TcpConnection 对象中 m_onConnection 存储的是 main 中 onConnection 函数的地址
        // main 函数中的 onConnection 函数通过回调在此处执行 --- 对应 void onConnection(TcpConnectionPtr conn), 
        // 所以 shared_from_this = TcpConnectionPtr
        // 故此处可以回调 main 中的 onConnection 函数

    }
    cout << "   -- TcpConnection::handleNewConnectionCallback Over" << endl;
}

void TcpConnection::handleMessageCallback()
{
    cout << "   TcpConnection::handleMessageCallback -- " << endl;
    if (m_onMessage) {
        m_onMessage(shared_from_this());
    }
    cout << "   -- TcpConnection::handleMessageCallback Over" << endl;
}

void TcpConnection::handleCloseCallback()
{
    cout << "   TcpConnection::handleCloseCallback -- " << endl;
    if (m_onClose) {
        m_onClose(shared_from_this());
    }
    cout << "   -- TcpConnection::handleCloseCallback Over" << endl;
}

void TcpConnection::sendInLoop(const string & msg)
{
    // 该函数中要将消息的发送打包成一个函数对象，注册给IO线程中的EventLoop对象，并且通知IO线程发送数据
    // 即 EventLoop::runInLoop 方法来执行该操作
    cout << "   TcpConnection::sendInLoop -- " << endl;
    if (m_loop) {
        m_loop->runInLoop(std::bind(&TcpConnection::send, this, msg));
    }
    cout << "   -- TcpConnection::sendInLoop Over" << endl;
}

int TcpConnection::readPacket(Packet & packet)
{
    cout << "   TcpConnection::readPacket -- " << endl;
    
    // 调用 SocketIO 读取客户端的packet信息
    return m_sockIO.readPacket(packet); 
    
    cout << "   -- TcpConnection::readPacket Over!" << endl;
}

void TcpConnection::sendInLoop(const TLV & data)
{
    cout << "   TcpConnection::sendInLoop(TLV) -- " << endl; 

    // 这个函数负责将TLV格式的数据发送给客户端

    int  tlvlen = 8 + data.length;
    string msg;                                 // 先构造空间
    msg.assign((const char *)&data, tlvlen);    // 再拷贝内容
    sendInLoop(msg);

    cout << "   -- TcpConnection::sendInLoop(TLV) Over!" << endl;
}



#if 0
void TcpConnection::sendAVPacket(AVPacket* pkt, AVStream* stream)
{
    cout << "   TcpConnection::sendAVPacket(AVPacket, AVStream) -- " << endl; 

    if (!pkt || !stream) {
        cerr << "  TcpConnection::sendAVPacket --  无效参数" << endl;
        return;
    }

    // 1. 构造帧头信息
    wdf::AVPacketHeader header{};
    header.stream_type = (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) ? 0 : 1;
    header.codec_id = stream->codecpar->codec_id;
    header.flags = (pkt->flags & AV_PKT_FLAG_KEY) ? 1 : 0; // 关键帧标记
    header.pts = pkt->pts;
    header.dts = pkt->dts;
    
    if (header.stream_type == 0) { // 视频
        header.width = stream->codecpar->width;
        header.height = stream->codecpar->height;
    } else { // 音频
        header.sample_rate = stream->codecpar->sample_rate;
        header.channels = stream->codecpar->channels;
    }

    // 2. 计算总长度
    const int header_size = sizeof(wdf::AVPacketHeader);
    const int data_size = pkt->size;
    const int total_size = header_size + data_size;

     // 3. 构造TLV消息
    std::vector<char> buffer(sizeof(int) * 2 + total_size); // type + length + data

    // 填充类型字段
    int type = wdf::TASK_TYPE_AV_PACKET;
    memcpy(buffer.data(), &type, sizeof(int));

    // 填充长度字段
    memcpy(buffer.data() + sizeof(int), &total_size, sizeof(int));

    // 填充帧头
    memcpy(buffer.data() + 2 * sizeof(int), &header, header_size);

    // 填充帧数据
    memcpy(buffer.data() + 2 * sizeof(int) + header_size, pkt->data, data_size);

    // 4. 发送数据
    this->sendInLoop(std::string(buffer.data(), buffer.size()));

    cout << "   -- TcpConnection::readPacket Over!" << endl;
}

#endif

InetAddress TcpConnection::getLocalAddress()
{
    // int getsockname(                         // 获取一个套接字（socket）的本地协议地址（IP地址和端口号）
    //                int sockfd,               // 已绑定或已连接的套接字文件描述符
    //                struct sockaddr *addr,    // 指向存放返回地址信息的缓冲区（通常为 struct sockaddr_in 或 struct sockaddr_in6）
    //                socklen_t *addrlen        // 输入时为缓冲区 addr 的大小，输出时为实际地址结构的长度
    //                );                        // 成功: 返回 0，且 addr 和 addrlen 被填充； 失败: 返回 -1

    cout << "   TcpConnection::getLocalAddress -- ";
    struct sockaddr_in addr;
    memset(&addr, 0 , sizeof(addr));
    socklen_t len = sizeof(addr);
    int ret = getsockname(m_sock.fd(), (struct sockaddr *)&addr, &len);
    if (ret < 0) {
         cerr << "getsockname failed: " << strerror(errno) << endl;
    } else {
        cout << "getsockname over!" << endl;
    }

    return InetAddress(addr);
}

InetAddress TcpConnection::getPeerAddress()
{
    // int getpeername(                             // 获取对端地址
    //                 int sockfd,                  // 已连接的套接字文件描述符（必须是面向连接的套接字，如TCP）
    //                 struct sockaddr *addr,       // 指向存放对端地址信息的缓冲区（通常用sockaddr_in或sockaddr_in6）
    //                 socklen_t *addrlen           // 输入时为缓冲区大小，输出时为实际地址结构长度（必须初始化）
    //                 );                           // 0	成功，addr 和 addrlen 已被填充; -1	失败，错误码存储在 errno 中

    cout << "   TcpConnection::getPeerAddress -- ";
    struct sockaddr_in addr;
    memset(&addr, 0 , sizeof(addr));
    socklen_t len = sizeof(addr);
    int ret = getpeername(m_sock.fd(), (struct sockaddr *)&addr, &len);
    if (ret < 0) {
         cerr << "getpeername failed: " << strerror(errno) << endl;
    } else {
        cout << "getpeername over!" << endl;
    }

    return InetAddress(addr);
}

}

