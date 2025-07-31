#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>      
#include <unistd.h>
#include <string.h>
#include <sys/select.h>
#include <sys/time.h>
#include <iostream>

using std::cout;
using std::cerr;
using std::endl;
using std::cin;
using std::string;

int main()
{
    // 创建 Socket
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientfd < 0) {
        cerr << "Socket 创建失败\n"; 
        return  -1;
    }
    cout << "Socket 创建成功\n";

    // Connect 服务端
    struct sockaddr_in client_addr;
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(8000);
    int ret = inet_pton(AF_INET, "127.0.0.1", &client_addr.sin_addr);
    if (ret != 1) {
        cerr << "IP地址转换失败\n";
        close(clientfd);
        return -1;
    }
    ret = connect(clientfd, (const struct sockaddr *)&client_addr, sizeof(client_addr));
    if (ret == -1) {
        cerr << "Connect 服务器失败\n";
        close(clientfd);
        return -1;
    }
    cout << "Connect 服务器成功\n";

    // int select(
    //     int nfds,                   // 最大文件描述符的值加一
    //     fd_set *readset,            // 结构中包含待检查是否有可读数据的文件描述符集合
    //     fd_set *writeset,           // 结构中包含待检查是否可以非阻塞写入的文件描述符集合
    //     fd_set *exceptionset,       // 结构中包含待检查是否有异常条件发生的文件描述符集合
    //     struct timeval * timeout    // 表示select调用的最长等待时间
    // );                              // 返回值:正数表示就绪的文件描述符数量, 0表示超时时间到了但没有文件描述符就绪, -1表示发生错误

    // struct timeval {
    //     long    tv_sec;         // seconds: 秒
    //     long    tv_usec;        // microseconds: 微秒 (1秒 = 1000 000微秒)
    // };

    //集合的相关操作如下：
    // void FD_ZERO(fd_set *fdset);            // 将所有fd清零
    // void FD_SET(int fd, fd_set *fdset);     // 增加一个fd
    // void FD_CLR(int fd, fd_set *fdset);     // 删除一个fd
    // int FD_ISSET(int fd, fd_set *fdset);    // 检查fd是否在fdset中被标记为“就绪”; 不在集合中返回零, 在则非零。

    // 通过 select 监听 clientfd 和 标准输入
    fd_set readset;

    // 循环通信 -- 发信息 -- 收信息
    char buffer[1024] = {};
    while (true) {
        FD_ZERO(&readset);
        FD_SET(clientfd, &readset);
        FD_SET(STDIN_FILENO, &readset);
        
        int nready = select(clientfd + 1, &readset, NULL, NULL, NULL);
        cout << "select nready = " << nready << endl;
        
        // 监听到标准输入
        if (FD_ISSET(STDIN_FILENO, &readset)) {
            memset(buffer, 0, sizeof(buffer));
            
            read(STDIN_FILENO, buffer, sizeof(buffer)); // read 接收数据会包含'\n'

            int bytes_sent = send(clientfd, buffer, strlen(buffer) - 1, 0); // strlen(buffer) - 1 不会发送'\n'
            if (bytes_sent == -1) {
                cerr << "Send 失败\n";
                break;
            }
            cout << "[客户端] send: " "(" << bytes_sent << ")字节 " << buffer << endl;

            // 客户端退出条件
            if (strcmp(buffer, "exit") == 0) {
                break;
            }
        }

        // 接收到服务端的数据
        // select 监视 client 时，当建立连接后，服务端先退出，会发送一个空数据给客户端, 客户端进入判断并退出
        if (FD_ISSET(clientfd, &readset)) {
            memset(buffer, 0, sizeof(buffer));

            int bytes_recevied = recv(clientfd, buffer, sizeof(buffer), 0);
            if (bytes_recevied == -1) {
                cerr << "接收数据失败\n";
                break;
            } else if (bytes_recevied == 0) {
                cerr << "服务器关闭连接\n";
                break;
            } 
            cout << "Recv 成功, [服务端]：" << buffer << endl;
            
            // 查看服务端是否要求退出
            if (strcmp(buffer, "exit") == 0) {
                cout << "服务端要求退出\n";
                break;
            }
        }
    }

    // 关闭连接
    close(clientfd);

    return 0;
}
