#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>      
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <iostream>

using std::cout;
using std::cerr;
using std::endl;
using std::cin;
using std::string;

struct TLV
{
    int type;
    int length;
    char data[1000];
};

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
    client_addr.sin_port = htons(8080);
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
    
    while (true) {
        cout << ">> input: ";
        string line;
        cin >> line;
        int id = 1;
        int len = line.length();
        
        // 测试: 发用户名
        send(clientfd, &id, 4, 0);
        send(clientfd, &len, 4, 0);
        send(clientfd, line.c_str(), line.size(), 0);
        cout << "[client] >> send " << 8 + len << " bytes." << endl;
        
        TLV tlv = {0};
        recv(clientfd, &tlv.type, 4, 0);
        recv(clientfd, &tlv.length, 4, 0);
        recv(clientfd, &tlv.data, tlv.length, 0);
        cout << "[client] << recv from server: " << tlv.data  << ", len: " << tlv.length << endl;
    }

    cout << "client is exiting !" << endl;
    // 关闭连接
    close(clientfd);

    return 0;
}
