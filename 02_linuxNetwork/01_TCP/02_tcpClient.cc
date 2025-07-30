#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>      
#include <unistd.h>
#include <string.h>
#include <iostream>

using std::cout;
using std::cerr;
using std::endl;

int main()
{   
    // int socket(
    //      int domain,     // 协议:AF_INET (IPv4)、AF_INET6 (IPV6)....
    //      int type,       // 套接字类型: SOCK_STREAM (TCP)、SOCK_DGRAM (UDP)....
    //      int protocol    // 协议:IPPROTO_TCP (TCP)、IPPTOTO_UDP (UDP)...; 当protocol为0时，会自动选择type类型对应的默认协议。
    // ); // 返回值:  返回值是一个非负整数, 代表一个文件描述符，用于标识创建的套接字，并通过这个描述符进行后续的网络I/O操作。
    
    // 1. 创建 Socket
    int clientfd = socket(AF_INET, SOCK_STREAM, 0); // socket函数本质是在内核态中创建了一个对象 --- 返回一个文件描述符来标识这个对象
                                                    // socket对象中, 包含了进行网络通信所需要的各种信息和状态(Eg:  地址族/Address Family,  类型/Type,  协议/Protocol,  地址/Socket Address ...)
                                                    // 这个对象中还维护了两个重要的缓冲区输入缓冲区和输出缓冲区, 这两个缓冲区分别用于临时存储从网络接收的数据和待发送到网络的数据    if (clientfd < 0) {
    if (clientfd < 0) {
        cerr << "Socket 创建失败\n";
        return -1;
    }
    cout << "Socket 创建成功\n";

    // ================================================================
    
    // 建立连接:  使用connect函数使客户端向服务器发送建立连接请求,初始化一个连接
    // int connect(
    //     int sockfd,                 // socket端点文件描述符
    //     const struct sockaddr *addr,// 目标服务器的地址和端口信息, 可以传 sockaddr_in、sockaddr_in6
    //     socklen_t addrlen           // 指定的addr代表结构体长度,确保bind函数可以正确解析给定的地址信息
    // ); // 返回值: 成功0, 失败-1

    // 2. 连接服务器
    struct sockaddr_in server_addr;                     // IPv4 专用结构体 sockaddr_in
    
    // struct sockaddr_in {
    //      sa_family_t    sin_family;  // 地址族（必须为 AF_INET）
    //      in_port_t      sin_port;    // 端口号（需用 htons() 转换字节序）16位
    //      struct in_addr sin_addr;    // IPv4 地址
    //      char           sin_zero[8]; // 填充字段（通常置 0）
    // };
    //
    // struct in_addr {
    //      uint32_t s_addr;            // 32 位 IPv4 地址（需用 inet_pton() 转换 或 htonl() 转换）
    // };
    
    // int inet_pton(                               // 将 点分十进制字符串（如 "127.0.0.1"） 转换为 二进制网络字节序（struct in_addr 或 struct in6_addr）
    //               int af,                        // 地址族：AF_INET（IPv4）或 AF_INET6（IPv6）  
    //               const char *restrict src,      // 点分十进制字符串（如 "192.168.1.1"）
    //               void *restrict dst             // 存储转换后的二进制地址（struct in_addr 或 struct in6_addr）
    //               );                             // 1 代表转换成功；0 代表src 不是有效的 IP 地址；-1	代表 af 不是 AF_INET 或 AF_INET6

    memset(&server_addr, 0, sizeof(server_addr));                           // 清空结构体
                                                                            //      1. 消除脏数据风险
                                                                            //      2. 满足填充字段要求（如 sin_zero）
    server_addr.sin_family = AF_INET;                                       // 设置 IPv4 地址族 
                                                                            //      1. IPv4 地址族，使用 32 位地址（如 192.168.1.1）; Pv6 地址族，使用 128 位地址（如 2001:0db8::1）
    server_addr.sin_port = htons(8000);                                     // 把端口号由主机字节序转为网络字节序
                                                                            //      1. 字节序：大端序（网络字节序采用此标准）与小端序（x86/ARM 等常见 CPU 使用此方式）
                                                                            //      2. htons()：Host to Network Short（16 位数据，如端口号）
                                                                            //      3. htonl()：Host to Network Long（32 位数据，如 IPv4 地址）
    int ret = inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);       // 将点分十进制 IP 转换为二进制网络字节序，并存入结构体
                                                                            //      1. inet_pton 成功时返回 1，失败返回 0（无效地址）或 -1（地址族不支持）
                                                                            //      2. inet_pton 支持 IPv4 (AF_INET) 和 IPv6 (AF_INET6),以及更安全的错误检查； 而 inet_adrr 只支持 IPv4
                                                                            //      3. inet_pton 已内置字节序处理，通常无需额外调用 htonl
    if (ret != 1) {
        cerr << "IP地址转换失败\n";
        close(clientfd);
        return -1;
    } 
    
    ret = connect(clientfd, (const struct sockaddr *)&server_addr, sizeof(server_addr));    // 调用 connect 的时机是完成 TCP 建立的三次握手
    if (ret == -1) {
        cerr << "连接服务器失败\n";
        close(clientfd);
        return -1;
    }

    cout << "服务器连接成功！\n";
    
    // ================================================================
    
    // 客户端使用 Recv用于接收TCP数据 -- 默认是阻塞的
    // ssize_t recv(
    //     int sockfd,     // socket端点文件描述符
    //     void *buf,      // 指向读出数据存放的缓冲区的指针
    //     size_t len,     // buf的长度，以字节为单位
    //     int flags       // 定接收行为的标志位:MSG_PEEK(查看数据但不从系统缓冲区中移除)、MSG_WAITALL(等待所有请求的数据才返回)...大多数情况下，flags设置为0。
    // ); // 返回值: 成功时返回实际读取的字节数。如果连接已经关闭返回0(对方close: 四次挥手)。读取失败返回-1

    // 3. 接受数据
    char buffer[1024] = {0};
    int bytes_received = recv(clientfd, buffer, sizeof(buffer), 0); // Send和 Recv函数只是将数据在用户态空间和内核态的缓冲区之间进行传输
    if (bytes_received == -1) {
        cerr << "接收数据失败\n";

    } else if (bytes_received == 0) {
        cout << "服务器关闭了连接\n";

    } else {
        cout << "服务器回复：" << buffer << "\n";

    }
    
    // 注意：
    //      TCP是一种流式的通信协议，消息是以字节流的方式在信道中传输，这就意味着一个重要的事情， 消息和消息之间是没有边界的
    //      1. 在不加额外约定的情况下，通信双方并不知道发送和接收到底有没有接收完一个消息，有可能多个消息会在一次传输中被发送和接收（江湖俗称"粘包"）
    //      2. 也有有可能一个消息需要多个传输才能被完整的发送和接收(江湖俗称"半包")
    
    close(clientfd);
    return 0;
}
