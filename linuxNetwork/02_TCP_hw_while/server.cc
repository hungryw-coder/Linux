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
using std::cin;
using std::string;

int main()
{
    // 创建 Socket
    int serverfd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverfd < 0) {
        cerr << "Socket 创建失败\n";
        return -1;
    }
    cout << "Socket 创建成功\n";
    
    // Bind 服务端
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8000);
    int ret = inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);
    if (ret != 1) {
        cerr << "IP地址转换失败\n";
        close(serverfd);
        return -1;
    }
    ret = bind(serverfd, (const struct sockaddr *)&server_addr, sizeof(server_addr));
    if (ret == -1) {
        cerr << "Bind 失败\n";
        close(serverfd);
        return -1;
    }
    cout << "Bind 成功\n";

    // Listen 客户端
    ret = listen(serverfd, 5);
    if (ret == -1) {
        cerr << "Listen 失败\n";
        close(serverfd);
        return -1;
    }
    cout << "Listen 成功\n";

    // Accept 客户端 
    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(client_addr));
    socklen_t client_addr_len = sizeof(client_addr);
    cout << "等待客户端的连接...\n"; 
    int clientfd = accept(serverfd, (struct sockaddr *)&client_addr, &client_addr_len);
    if (clientfd < 0) {
        cerr << "Accept 失败\n";
        close(serverfd);
        return -1;
    }
    cout << "客户端连接成功\n";

    // 通信循环 -- recv 数据 -- send 回复
    char buffer[1024];
    while (true) {
        // 接收数据
        memset(buffer, 0, sizeof(buffer));
        int bytes_recevied = recv(clientfd, buffer, sizeof(buffer), 0);
        if (bytes_recevied == -1) {
            cerr << "接收数据失败\n";
            break;
        } else if (bytes_recevied == 0) {
            cerr << "服务器关闭连接\n";
            break;
        } 
        cout << "Recv 成功\n";
        cout << "客户端：" << buffer << endl;

        // 检查退出条件
        if (strcmp(buffer, "exit") == 0) {
            cout << "客户端请求退出\n";
            break;
        }

        // 发送回复
        cout << "[服务端] reply 信息：";
        string reply;
        getline(cin, reply);
        int bytes_sent = send(clientfd, reply.c_str(), reply.length(), 0);
        if (bytes_sent == -1) {
            cerr << "Send 失败\n";
            break;
        }
        cout << "Send 成功\n";

        // 服务器退出条件
        if (reply == "exit") {
            break;
        }
    }

    // 关闭连接
    close(clientfd);
    close(serverfd);

    return 0;
}

