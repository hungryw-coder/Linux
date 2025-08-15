#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>      
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <iostream>
#include <shadow.h>
#include <crypt.h>

using std::cout;
using std::cerr;
using std::endl;
using std::cin;
using std::string;

struct TLV {
    int type;
    int length;
    char data[1000];
};

int main() {
    // 创建 Socket
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientfd < 0) {
        cerr << "Socket 创建失败\n"; 
        return -1;
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

    // 第一阶段：发送用户名
    cout << "请输入用户名: ";
    string username;
    cin >> username;
    
    TLV tlv_send;
    tlv_send.type = 1; // TASK_TYPE_LOGIN_SECTION1
    tlv_send.length = username.length();
    strncpy(tlv_send.data, username.c_str(), tlv_send.length);
    
    send(clientfd, &tlv_send.type, 4, 0);
    send(clientfd, &tlv_send.length, 4, 0);
    send(clientfd, tlv_send.data, tlv_send.length, 0);
    cout << "[client] >> 发送用户名: " << username << endl;

    // 接收服务端返回的盐值
    TLV tlv_recv;
    recv(clientfd, &tlv_recv.type, 4, 0);
    recv(clientfd, &tlv_recv.length, 4, 0);
    recv(clientfd, tlv_recv.data, tlv_recv.length, 0);
    
    if (tlv_recv.type == 3) { // TASK_TYPE_LOGIN_SECTION1_RESP_ERROR
        cerr << "用户名错误或不存在" << endl;
        close(clientfd);
        return -1;
    } else {
        cout << "用户名存在" << endl;
    }
    
    string setting(tlv_recv.data, tlv_recv.length);
    cout << "[client] << 收到盐值: " << setting << endl;

    // 第二阶段：发送加密后的密码
    cout << "请输入密码: ";
    string password;
    cin >> password;
    
    // 使用盐值加密密码
    char* encrypted = crypt(password.c_str(), setting.c_str());
    if (!encrypted) {
        cerr << "密码加密失败" << endl;
        close(clientfd);
        return -1;
    } else {
        cout << "密码加密成功" << endl;
    }
    
    tlv_send.type = 4; // TASK_TYPE_LOGIN_SECTION2
    tlv_send.length = strlen(encrypted);
    strncpy(tlv_send.data, encrypted, tlv_send.length);
    
    send(clientfd, &tlv_send.type, 4, 0);
    send(clientfd, &tlv_send.length, 4, 0);
    send(clientfd, tlv_send.data, tlv_send.length, 0);
    cout << "[client] >> 发送加密密码: " << encrypted << endl;

    // 接收验证结果
    recv(clientfd, &tlv_recv.type, 4, 0);
    recv(clientfd, &tlv_recv.length, 4, 0);
    recv(clientfd, tlv_recv.data, tlv_recv.length, 0);
    
    if (tlv_recv.type == 5) { // TASK_TYPE_LOGIN_SECTION2_RESP_OK
        cout << "[client] << 登录成功!" << endl;
    } else {
        cout << "[client] << 登录失败!" << endl;
    }

    close(clientfd);
    return 0;
}
