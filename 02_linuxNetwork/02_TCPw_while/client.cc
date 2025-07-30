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

    // 循环通信 -- 发信息 -- 收信息
    char buffer[1024];
    while (true) {
        // 发送信息
        cout << "[客户端] send 信息：";
        string msg;
        getline(cin, msg);
        int bytes_sent = send(clientfd, msg.c_str(), msg.length(), 0);
        if (bytes_sent == -1) {
            cerr << "Send 失败\n";
            break;
        }
        cout << "Send 成功\n";

        // 客户端退出条件
        if (msg == "exit") {
            break;
        }

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
        cout << "服务端：" << buffer << endl;
        
        // 查看服务端是否要求退出
        if (strcmp(buffer, "exit") == 0) {
            cout << "服务端要求退出\n";
            break;
        }
    }

    // 关闭连接
    close(clientfd);

    return 0;
}
