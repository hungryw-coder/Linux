#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>      
#include <unistd.h>
#include <string.h>
#include <iostream>

using std::cout;
using std::cerr;
using std::endl;
using std::cin;
using std::string;

#define MAX_EVENT_SIZE 100  // 最大事件数
#define EPOLL_TIMEOUT 5000  // 超时时间 5 秒

int main()
{
    // 1. 创建 TCP Socket 
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        cerr << "Socket 创建失败: " << strerror(errno) << endl;
        return -1;
    }

    // 避免地址被占用的问题
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 2. Bind 网络地址
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    int ret = inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);
    if (ret != 1) {
        cerr << "IP地址转换失败: " << strerror(errno) << endl;
        close(server_fd);
        return -1;
    }
    ret = bind(server_fd, (const struct sockaddr *)&server_addr, sizeof(server_addr));
    if (ret == -1) {
        cerr << "Bind 失败: " << strerror(errno) << endl;
        close(server_fd);
        return -1;
    }
    cout << "Bind 成功\n";

    // 3. Listen 新链接到来
    ret = listen(server_fd, 1);
    if (ret == -1) {
        cerr << "Listen 失败: " << strerror(errno) << endl;
        close(server_fd);
        return -1;
    }
    cout << "Listen 成功\n";

    // ==================================================
    
    // epoll_create1 - 创建 epoll 实例
    // int epoll_create1(              
    //                  int flags       // 控制 epoll 实例的行为，通常为 0 或 EPOLL_CLOEXEC（关闭 exec 时自动关闭 fd）
    //                  );              // epfd (成功)	返回一个新的 epoll 文件描述符（epfd），用于后续操作
    //                                  // -1 (失败)	返回 -1 并设置 errno（如 EMFILE 表示进程 fd 数已达上限）

    // 4. 创建 epoll 实例
    int epfd = epoll_create1(0);
    if (epfd == -1) {
        cerr << "epoll_create1 失败: " << strerror(errno) << endl;
        close(server_fd);
        return -1;
    } 
    cout << "epoll 实例创建成功, epfd = " << epfd << endl;

    // 此时，内核会为 epfd 分配一个 struct eventpoll 结构体（包含红黑树和就绪链表）
    
    // ==================================================
    
    // epoll_ctl - 管理监听的 fd
    // int epoll_ctl(                                          
    //               int epfd,                                 // epoll_create1 返回的 epoll 实例 fd
    //               int op,                                   // EPOLL_CTL_ADD（添加 fd）; EPOLL_CTL_MOD（修改 fd）; EPOLL_CTL_DEL（删除 fd）
    //               int fd,                                   // 需要监听的目标文件描述符（如 socket fd）
    //               struct epoll_event *_Nullable event       // 监听的事件配置（见下方结构体说明）。EPOLL_CTL_DEL 时可设为 NULL
    //               );                                        // 0 (成功)	操作成功, -1 (失败)	返回 -1 并设置 errno（如 EBADF 表示 epfd 无效）
    
    // struct epoll_event {
    //     uint32_t     events;  // 监听的事件类型（位掩码）
    //     epoll_data_t data;    // 用户数据（通常关联 fd）
    // };

    // typedef union epoll_data {
    //     void    *ptr;
    //     int      fd;         // 常用：关联的 fd
    //     uint32_t u32;
    //     uint64_t u64;
    // } epoll_data_t;
    
    // 常用事件类型（events 字段）：
    //      - EPOLLIN	数据可读（如 socket 接收缓冲区非空）
    //      - EPOLLOUT	数据可写（如 socket 发送缓冲区未满）
    //      - EPOLLET	边缘触发模式（默认是水平触发 LT）
    //      - EPOLLRDHUP	对端关闭连接或半关闭（需内核 2.6.17+）
    //      - EPOLLONESHOT	事件只触发一次，需重新注册

    // 5. 添加 server_fd 到 epoll 监听
    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.data.fd = server_fd;
    cout << "server_fd = " << server_fd << endl;
    ev.events = EPOLLIN;                                        // 对server_fd上的读事件进行监听
    ret = epoll_ctl(epfd, EPOLL_CTL_ADD, server_fd, &ev);
    if (ret == -1) {
        cerr << "epoll_ctl 失败: " << strerror(errno) << endl;
        close(server_fd);
        close(epfd);
        return -1;
    }
    cout << "添加 server_fd 到 epoll 监听\n";
    
    // epoll_ctl 向 epfd 添加要监听的 fd 及其事件EPOLLIN
    // 内核将 fd 插入红黑树，并关联回调函数（当 fd 就绪时，回调将其加入就绪链表）

    // ==================================================
    
    // epoll_wait - 等待事件就绪
    // int epoll_wait (
    //                int epfd,                        // epoll 实例 fd
    //                struct epoll_event *events,      // 输出参数，存放就绪事件的数组（由调用者预先分配）
    //                int maxevents,                   // events 数组的最大长度（必须 > 0）
    //                int timeout                      // 超时时间（毫秒）: -1：阻塞等待; 0：立即返回; >0：超时时间
    //                ); // >0 (成功)	返回就绪的事件数量（填充到 events 数组中）; 0 (超时)	超时且无事件就绪; -1 (失败)	返回 -1 并设置 errno（如 EINTR 被信号中断）
    
    // 6. 事件循环 -- 不断对事件进行监听
    
    // epoll_wait 检查就绪链表
    //      - 若链表非空，拷贝就绪事件到用户空间 events 数组并返回数量
    //      - 若链表为空，根据 timeout 阻塞或超时返回
    
    cout << "开始进入epoll监听\n";
    struct epoll_event eventsArr[MAX_EVENT_SIZE] = {};                              // 所有元素默认初始化为 0, 符合现代 C++ 风格
    while (true) {
        int nready = epoll_wait(epfd, eventsArr, MAX_EVENT_SIZE, EPOLL_TIMEOUT);
        if (nready == -1 && EINTR == errno) {

            // 当epoll_wait()返回-1时，通常表示发生了错误
            // 但是有一种特殊情况：如果进程在执行epoll_wait()时被信号中断（即收到了一个信号），epoll_wait()会返回-1，并且设置errno为EINTR
            // 这种情况下，这并不是真正的错误，而是被信号打断的正常情况
            
            cout << "epoll_wait 被信号中断，重新调用epoll_wait()继续等待事件\n";
            continue;
        } else if (nready == -1 ) {
            cerr << "epoll_wait 失败: " << strerror(errno) << endl; 
            break;
        } else if (nready == 0) {
            cout << "timeout 且无事件就绪\n";
        } else {
            cout << "就绪事件数 nready = " << nready << endl;
        } 
        
        // nready > 0 时，悠文件描述符就绪
        for (int i = 0; i < nready; ++i) {
            // 遍历数组获取就绪的文件描述符
            int fd = eventsArr[i].data.fd;
            cout << "fd = " << fd << endl;
            
            // 判断新连接 -- if (fd == server_fd)
            // 这个判断成立的原因：
            //      - 1. 当前只对 server_fd 注册了 EPOLLIN 事件, 这个事件只会在监听套接字上有新连接到达时触发
            //      - 2. 监听套接字 (server_fd) 的唯一作用就是接受新连接, 当客户端调用 connect() 时，监听套接字就会变为"可读"状态
            //      - 3. epoll_wait() 返回的就绪事件数组中, 只有监听套接字会触发"有新连接"的事件, 普通客户端套接字触发的是"有数据可读"的事件
            
            // Q1: 普通套接字也能触发 "新连接" 事件？
            // 🙅 错误的！
            //      1. >>> 只有 listen() 过的套接字才会触发 EPOLLIN 作为 "新连接" 事件 <<< 
            //      2. 普通套接字的 EPOLLIN 仅表示 "数据可读"
            //
            // Q2: accept() 会阻塞？
            //      1. 在 非阻塞模式 + epoll 下，accept() 只有在 ACCEPT 队列非空时才会被调用，因此不会阻塞
            //      2. 如果 epoll_wait() 返回监听套接字的 EPOLLIN，则 accept() 一定能立即返回
            //  Q2解析：
            //      1. 默认情况：accept() 的阻塞行为 --- 如果 ACCEPT 队列（全连接队列--完成三次握手）为空（没有新连接），accept() 会一直阻塞，直到有新连接到来
            //      2. 非阻塞模式下的 accept() --- 通过 fcntl 设置监听套接字为非阻塞：fcntl(server_fd, F_SETFL, O_NONBLOCK);  
            //          - 如果 ACCEPT 队列为空，accept() 会立即返回 -1，并设置 errno = EAGAIN 或 EWOULDBLOCK表示无连接
            //          - 必须配合 I/O 多路复用（如 epoll） 才能高效工作 --- 只在 ACCEPT 队列非空时调用，不会阻塞

            // 处理新链接
            if (fd == server_fd) {  // fd = server_fd = 3  只有监听套接字会触发 "新连接" 事件
                
                struct sockaddr_in client_addr;
                socklen_t len = sizeof(client_addr);
                int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &len); // 将 client_fd 加入 epoll 监听（关注 EPOLLIN 数据可读）
                if (client_fd < 0) {
                    cerr << "ACcept 失败: " << strerror(errno) << endl;
                    continue;
                }
            
                // 新连接信息打印
                char client_ip[INET_ADDRSTRLEN];
                const char * convert_ptr = inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
                if (convert_ptr != NULL) {  
                    cout << "新的客户端连接: " << client_fd << "  " << client_ip << ":" << ntohs(client_addr.sin_port) << "  has connected"<< endl;
                } else {
                    cerr << "inet_ntop 转换失败: " << strerror(errno) << endl;
                    continue;
                }
                
                // epoll 对 client_fd 进行读事件监听
                ev.data.fd = client_fd;
                ev.events = EPOLLIN;                                    // 对 client_fd 上的读事件进行监听
                ret = epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &ev);
                if (ret == -1) {
                    cerr << "Failed to add client fd: " << strerror(errno) << endl;
                    close(client_fd);
                    continue;
                }
                cout << "添加 client_fd = " << client_fd  << " 到 epoll 监听" << endl;
                       
            } else {
                // 普通套接字触发的是 "数据可读" 事件
                
                // client_fd 已建立好的连接上有读事件就绪
                char buffer[1024] = {};
                
                // 接收数据 
                int bytes_received = recv(fd, buffer, sizeof(buffer), 0);
                if (bytes_received == 0) {
                    // 连接断开 -- 从 epoll 监听的红黑树上删除该文件描述符
                    ev.data.fd = fd;
                    ret = epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &ev);
                    if (ret == -1) {
                        cerr << "epoll_ctl 失败: " << strerror(errno) << endl;
                        continue;
                    }
                    cout << "删除 就绪数组中已断开连接的 fd \n";
                    close(fd);
                    continue;
                } else if (bytes_received == -1) {
                    cerr << "recv 失败: " << strerror(errno) << endl;
                    continue;
                }

                // 数据接收成功
                cout << "[Server] recv: " << buffer << "(" << bytes_received << "字节)" << endl; 

                // 服务器回复客户端信息
                cout << "[Server] reply: ";
                string reply;
                getline(cin, reply);
                int bytes_sent = send(fd, reply.c_str(), reply.length(), 0);
                if (bytes_sent < 0) {
                    cerr << "Send 失败: " << strerror(errno) << endl;
                    continue;
                }
                cout << "[Server] send: " << bytes_sent << "字节\n";
            }

        } // for1

    }// while(1)


    // 关闭连接
    close(epfd);
    close(server_fd);
    return 0;
}

