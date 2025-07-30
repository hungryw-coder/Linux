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
    // 初始化Socket:  调用Socket函数创建一个Socket通信端点 
    // int socket(
    //      int domain,     // 协议:AF_INET (IPv4)、AF_INET6 (IPV6)....
    //      int type,       // 套接字类型: SOCK_STREAM (TCP)、SOCK_DGRAM (UDP)....
    //      int protocol    // 协议:IPPROTO_TCP (TCP)、IPPTOTO_UDP (UDP)...; 当protocol为0时，会自动选择type类型对应的默认协议。
    // ); // 返回值:  返回值是一个非负整数, 代表一个文件描述符，用于标识创建的套接字，并通过这个描述符进行后续的网络I/O操作。
    
    // 1. 创建 Socket
    int serverfd = socket(AF_INET, SOCK_STREAM, 0); // >>> 监听套接字 <<<
    if (serverfd < 0 ) {
        cerr << "Socket 创建失败\n";
        return -1;
    }
    cout << "Socket 创建成功\n";
    
    // ================================================================

    // 使用bind函数给socket端点绑定端口和IP
    // int bind(
    //     int sockfd,                          // socket端点文件描述符
    //     const struct sockaddr *addr,         // 要绑定的IP地址和端口号, 实际使用sockaddr_in  (IPv4)，sockaddr_in6  (IPv6)
    //     socklen_t addrlen                    // 指定的addr代表结构体长度,确保bind函数可以正确解析给定的地址信息:sizeod(addr)
    // ); //返回值: 成功时返回0。失败返回-1

    // 2. 绑定 IP 和端口
    struct sockaddr_in  server_addr;

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
        close(serverfd);
        return -1;
    }

    ret = bind(serverfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)); // 开始绑定
    if (ret == -1) {
        cerr << "Bind 失败\n";
        close(serverfd);
        return -1;
    }
    cout << "Bind 成功\n";

    // ================================================================

    // 设置监听: 使用listen函数对设置好端口和IP的服务端socket端点监听外部连接请求
    // int listen(
    //     int sockfd,     // socket端点文件描述符
    //     int backlog     // 这个参数指定了套接字可以挂起的最大连接数
    //                     // backlog参数实际上是用来设置Socket的ACCEPT队列/全连接队列的最大长度(在有的操作系统上backlog指的是半连接队列和全连接队列的长度之和) 
    //                     // 需要注意的是, 如果队列已经满了，那么服务端受到任何再发起的连接都会直接丢弃（大部分操作系统中服务端不会回复，以方便客户端自动重传）
    // ); //返回值: 成功返回0, 失败返回-1

    // 3. 监听连接
    ret = listen(serverfd, 5); // 最大等待 5 个连接
    if (ret == -1) {
        cerr << "Listen 失败\n";
        close(serverfd);
        return -1;
    }
    cout << "Listen 成功\n";
    // 一旦启用了listen之后，操作系统就知道该套接字是服务端的套接字, 
    // 操作系统内核就不再启用其发送和接收缓冲区(回收空间)，转而在内核区维护两个队列结构： 半连接队列和全连接队列
    //      1. 半连接队列用于管理成功第一次握手的连接
    //      2. 全连接队列用于管理已经完成三次握手的队列
   
    // ================================================================

    // 获取连接: 使用accept函数从服务端的socket端点的全连接队列中取出一个连接 
    // int accept(
    //     int sockfd,             // socket端点文件描述符
    //     struct sockaddr *addr,  // 用来获取连接对端/客户端的地址信息。如果不需要对端的地址信息, 可设参数为NULL
    //     socklen_t *addrlen      // 用来获取addr结构体的大小。如果使用addr/非NULL,那么addrlen必须设置addr的大小/sizeof(addr);  如果addr是NULL，addrlen也必须是NULL。
    // ); // 返回值: 成功则返回一个新的套接字文件描述符,用于与客户端通信。失败返回-1。

    // const char *inet_ntop(                              // 将 二进制网络字节序（struct in_addr 或 struct in6_addr） 转换为 点分十进制字符串（如 "192.168.1.1"）
    //                       int af,                       // 地址族：AF_INET（IPv4）或 AF_INET6（IPv6）
    //                       const void *restrict src,     // 二进制地址（struct in_addr 或 struct in6_addr）
    //                       char dst[restrict .size],     // 存储转换后的字符串（必须足够大）
    //                       socklen_t size                // dst 缓冲区的大小（防止溢出）
    //                       ); // dst（非 NULL）成功，返回 dst 指针; NULL	失败（errno 会设置）

    // 4. 接受客户端连接
    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(client_addr));
    socklen_t client_len = sizeof(client_addr);
    
    // - accept 函数由服务端调用，用于从全连接队列中取出下一个已经完成的TCP连接（三次握手）
    // - 如果全连接队列为空（没有新的客户端成功三次握手），那么accept会陷入阻塞; 一旦全连接队列中到来新的连接，此时accept操作就会就绪 (注意: 这种就绪是读就绪) 

    cout << "等待客户端连接...\n";
    int clientfd =  accept(serverfd, (struct sockaddr *)&client_addr, &client_len); // >>>已连接套接字<<<
    if (clientfd == -1) {
        cerr << "Accept 失败\n";
        close(clientfd);
        close(serverfd);
        return  -1;
    }
    cout << "客户端连接成功\n";
    // - 当accept执行完了之后，内核会创建一个新的套接字文件对象，该文件对象关联的文件描述符是accept的返回值
    // - 文件对象当中最重要的结构是一个发送缓冲区和接收缓冲区，可以用于服务端通过TCP连接发送和接收TCP段
    
    // ps:注意区分两个套接字对象:   
    //    1. 通过把旧的管理连接队列的套接字称作监听套接字，而新的用于发送和接收TCP段的套接字称作已连接套接字
    //    2. 通常来说，监听套接字会一直存在，负责建立各个不同的TCP连接(只要源IP、源端口、目的IP、目的端口四元组任意一个 字段有区别，就是一个新的TCP连接)
    //       而某一条单独的TCP连接则是由其对应的已连接套接字进行数据通信的
    
    char client_ip[INET_ADDRSTRLEN];                                                        //   INET_ADDRSTRLEN = 16（IPv4 最大长度）
    const char * ret_ptr =  inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));                // 将二进制网络字节序转化为十进制 IP
    if (ret_ptr != NULL) {
        cout << "客户端连接：" << client_ip << ":" << ntohs(client_addr.sin_port) << "\n";      // 将端口号由网络字节序（大端序）转为主机字节序（小端序）
    } else {
        cerr << "inet_ntop 失败\n";
        close(serverfd);
        return -1;
    }
    
    // ================================================================
    
    // 服务端使用 Send 用于发送TCP数据 -- 默认是阻塞的
    // ssize_t send(
    //     int sockfd,         // socket端点文件描述符
    //     const void *buf,    // 指向要发送数据的缓冲区的指针
    //     size_t len,         // buf中数据的长度，以字节为单位
    //     int flags           // 用于指定发送操作的额外选项: MSG_OOB(发送紧急数据)、MSG_DONTROUTE(不经过路由器直接发送到本地网络上的目的地)...大多数情况下，flags参数设置为0。
    // );// 返回值: 成功返回实际发送的字节数。失败返回-1

    // 5. 发送数据
    const char * msg = "Hello from server!";
    int bytes_sent = send(clientfd, msg, strlen(msg), 0);
    if (bytes_sent == -1) {
        cerr << "Send 失败\n";
    } else {
        cout << "Send 成功, msg = " << msg << "\n";
    }
    // Send和 Recv函数只是将数据在用户态空间和内核态的缓冲区之间进行传输
    // send时将数据拷贝到内核态并不意味着会马上传输，而是由操作系统决定根据合适的时机,  再由内核协议栈按照协议的规范进行分节发送

    // 注意：
    //      TCP是一种流式的通信协议，消息是以字节流的方式在信道中传输，这就意味着一个重要的事情， 消息和消息之间是没有边界的
    //      1. 在不加额外约定的情况下，通信双方并不知道发送和接收到底有没有接收完一个消息，有可能多个消息会在一次传输中被发送和接收（江湖俗称"粘包"）
    //      2. 也有有可能一个消息需要多个传输才能被完整的发送和接收(江湖俗称"半包")
    
    close(clientfd);
    close(serverfd);
    return 0;
}

