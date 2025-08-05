#include <sys/socket.h>
#include <sys/epoll.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <map>
#include <vector>
#include <iostream>

using std::cout;
using std::cerr;
using std::endl;
using std::cin;
using std::string;
using std::map;
using std::vector;

#define MAX_EVENT_SIZE 100  // 最大事件数
#define EPOLL_TIMEOUT 5000  // 超时时间 5 秒
#define INACTIVE_TIMEOUT 30 // 客户端不活跃超时时间 30 秒

struct ClientInfo 
{
    time_t last_active_time;    // 最后活跃时间
    string ip;                  // 客户端IP
    int port;                   // 客户端端口
};

int main() 
{
    cout << "悠悠大王聊天室 Starting ... " << endl;
    
    // 创建Socket
    // 避免ip地址被占用
    // 绑定服务端地址
    // 监听新链接
    // 创建 epoll 实例 -- epoll_create1
    // 添加服务端监听套接字到 epoll 监听 -- epoll_ctl
    // 开启事件循环
    // 等待事件就绪 -- epoll_wait
    //      处理新连接：listen 过的 Socket 才会触发新连接事件; 接收对端信息; 对新接收的客户端进行事件监听
    //      处理已连接 Socket 
    // 关闭文件描述符
    
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        cerr << "Socket 创建失败: " << strerror(errno) << endl;
        return -1;
    }
    cout << "Server socket created" << endl;
    
    int opt = 1;
    int ret = setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if (ret == -1) {
        cerr << "setsockopt -- reuseAddr 失败" << endl;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    ret = inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);
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
    cout << "bind server addr ready" << endl;

    ret = listen(server_fd, 10);
    if (ret == -1) {
        cerr << "Listen 失败: " << strerror(errno) << endl;
        close(server_fd);
        return -1;
    }
    cout << "listen ready" << endl;      

    int epfd = epoll_create1(0);
    if (epfd == -1) {
        cerr << "epoll_create1 失败: " << strerror(errno) << endl;
        close(server_fd);
        return -1;
    }
    cout << "epoll_create1 over, epfd = " << epfd << endl;
    
    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.data.fd = server_fd;     // 获取用户数据 -- 关联fd
    ev.events = EPOLLIN;        // 监听事件为读事件
    epoll_ctl(epfd, EPOLL_CTL_ADD, server_fd, &ev);
    if (ret == -1) {
        cerr << "epoll_ctl 失败: " << strerror(errno) << endl;
        close(server_fd);
        close(epfd);
        return -1;
    }
    cout << "epoll_ctl over, epfd 添加了要监听的 server_fd = " << server_fd << ", 及 server_fd 的读事件" << endl
         << "内核将 server_fd 插入红黑树，并关联回调函数(当 server_fd 就绪时， 回调将其加入就绪链表)" << endl;

    // ===== 存储客户端fd信息和最后活跃时间 ====
    map<int, ClientInfo> clients;

    cout << ">>> 开启事件循环，等待事件就绪" << endl;
    struct epoll_event eventsArr[MAX_EVENT_SIZE];
    while (true) {
        int nready = epoll_wait(epfd, eventsArr, MAX_EVENT_SIZE ,EPOLL_TIMEOUT);
        if (nready == -1 && EINTR == errno) {
            cout << "epoll_wait 被信号中断，重新调用epoll_wait()继续等待事件\n";
            continue;
        } else if (nready == -1 ) {
            cerr << "epoll_wait 失败: " << strerror(errno) << endl; 
            break;
        } else if (nready == 0) {
            cout << "epoll_wait 检查就绪链表为空，无事件就绪， timeout" << endl;
        } else {
            cout << "[Sys] epoll_wait 检查就绪链表非空，拷贝就绪事件到用户空间 eventsArr  中并返回数量" << endl
                 << "就绪事件数 nready = " << nready << endl;
        
            // ==== 检查不活跃的客户端 ===
            time_t current_time = time(nullptr);    // 当前时间
            vector<int> inactive_clients;           // 保存将要踢出的客户端 

            for (const auto & client : clients) {
                cout << "[inspect] >>> out inspect\n";

                int timeDistance = current_time - client.second.last_active_time; 
                if (timeDistance > INACTIVE_TIMEOUT) {
                    cout << "[客户端] " << client.first << "(" << client.second.ip << ":" << client.second.port << ") 超过 " 
                         << INACTIVE_TIMEOUT << " 秒未活跃，将被踢出聊天室" << endl;  
                    
                    // ==== Save map<int, ClientInfo> 中不活跃的客户端 ====
                    inactive_clients.push_back(client.first);
                }

                // ==== 开始踢出不活跃用户 ====
                for (int fd : inactive_clients) {
                    // 从 epoll 中删除
                    ev.data.fd = fd;
                    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &ev);

                    // 关闭连接
                    close(fd);

                    // 从客户端列表中删除
                    clients.erase(fd);
                    cout << "had out inactive client: " << fd << endl;
                }

            }

            for (int i = 0; i < nready; ++i) {
                cout << " \n======= 开始处理就绪事件" << endl;
                int fd = eventsArr[i].data.fd;  // fd来自epoll_ctl添加的事件
                cout << "epoll_event events current fd = " << fd << endl;

                if (fd == server_fd) {
                    cout << "开始处理新连接, new connection fd = " << fd << ", server_fd = " << server_fd << endl;
                    
                    struct sockaddr_in client_addr;
                    socklen_t addr_len =  sizeof(client_addr);
                    int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
                    if (client_fd < 0) {
                        cerr << "ACcept 失败: " << strerror(errno) << endl;
                        continue;
                    }
                    
                    char client_ipBuff[INET_ADDRSTRLEN];
                    const char * newConnection = inet_ntop(AF_INET, &client_addr.sin_addr, client_ipBuff, sizeof(client_ipBuff));
                    if (newConnection != NULL) {
                        cout << "新的客户端连接: client_fd = " << client_fd << "  " << client_ipBuff << ":" << ntohs(client_addr.sin_port) << "  has connected"<< endl;
                        
                        // ========记录客户端信息=============
                        ClientInfo info;
                        info.last_active_time = time(nullptr);
                        info.ip = client_ipBuff;
                        info.port = ntohs(client_addr.sin_port);
                        clients[client_fd] = info;
                        cout << ">>> 已记录客户端信息到客户端列表" << endl;
                    } else {
                        cerr << "inet_ntop 转换失败: " << strerror(errno) << endl;
                        continue;
                    }

                    ev.data.fd = client_fd;     // 关联fd，获取用户数据
                    ev.events = EPOLLIN;        // 监听普通套接字的读事件
                    ret = epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &ev);
                    if (ret == -1) {
                        cerr << "Failed to add client fd: " << strerror(errno) << endl;
                        close(client_fd);
                        continue;
                    }
                    cout << "epoll_ctl over, epfd 添加了要监听的 client_fd = " << client_fd << "及 client_fd 的读事件" << endl
                         << "内核将 client_fd 插入红黑树，并关联回调函数(当 client_fd 就绪时， 回调将其加入就绪链表)" << endl;
                
                } else {
                    cout << " \n========= 开始处理已连接的连接 " << endl;
                    
                    char buff[1024] = {};
                    int bytes_received = recv(fd, buff, sizeof(buff), 0);
                    if (bytes_received == 0) {
                        cout << "对端断开连接" << endl;

                        ev.data.fd = fd;
                        ret = epoll_ctl(epfd, EPOLL_CTL_DEL,  fd, &ev);
                        if (ret == -1) {
                            cerr << "epoll_ctl 失败: " << strerror(errno) << endl;
                            continue;
                        }
                        cout << "epoll_ctl detelet over, epfd 删除当前关闭的对端文件描述符 client_fd = " << fd << endl
                             << "内核将已关闭的对端 client_fd 从红黑树中删除" << endl;
                        
                        close(fd);

                        // ==== 主动断开连接，删除clients中的该不活跃信息 ====
                        clients.erase(fd);
                        continue;
                    }

                    // ==== 更新最后活动时间 ====
                    clients[fd].last_active_time = time(nullptr);     
                    
                    // ==== 处理接收的信息 =====
                    buff[bytes_received] = '\0';        // 将换行符转换为字符串结束符'\0'
                    cout << "[Server] recv: " << clients[fd].ip << ":" << clients[fd].port << " 的信息: " << buff << "(" <<  bytes_received << "字节)" << endl;
                    
                    // ==== 转发信息给所有客户端 ==== 
                    string msg = string("[User ]") + clients[fd].ip + ":" + std::to_string(clients[fd].port) + "] " + buff;
                    for (const auto & client : clients) {
                        if (client.first != fd) {   // 不转发信息给自己
                            int bytes_sent = send(client.first, msg.c_str(), msg.length(), 0);
                            if (bytes_sent < 0) {
                                cerr << "Send 失败: " << strerror(errno) << endl;
                            }
                        }
                    } // for
        
                } // else       
            } // for
        } // else
    } // while 
    
    close (epfd);
    close(server_fd);
    return 0;
}
