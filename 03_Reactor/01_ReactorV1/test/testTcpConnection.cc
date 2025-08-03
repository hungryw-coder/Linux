#include "../TcpConnection.hpp"
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>

// 测试基本发送接收
void test_basic_communication() {
    std::cout << "===== 测试基本收发 =====" << std::endl;
    
    // 创建socket对
    int fds[2];
    socketpair(AF_UNIX, SOCK_STREAM, 0, fds);
    
    // 创建TcpConnection对象（使用fd[0]）
    wdf::TcpConnection conn(fds[0]);
    
    // 测试发送
    conn.send("hello");
    char buf[10];
    int n = read(fds[1], buf, sizeof(buf));
    std::cout << "对端收到: " << std::string(buf, n) << std::endl;
    
    // 测试接收
    write(fds[1], "world\n", 6);
    std::string received = conn.receive();
    std::cout << "接收到: " << received;
    
    close(fds[0]);
    close(fds[1]);
}

// 测试连接关闭检测
void test_connection_close() {
    std::cout << "\n===== 测试连接关闭 =====" << std::endl;
    
    int fds[2];
    socketpair(AF_UNIX, SOCK_STREAM, 0, fds);
    
    wdf::TcpConnection conn(fds[0]);
    
    // 关闭对端
    close(fds[1]);
    
    // 检查连接状态
    std::cout << "连接是否关闭: " << (conn.isClosed() ? "是" : "否") << std::endl;
    
    // 尝试接收（应返回空字符串）
    std::cout << "接收数据: \"" << conn.receive() << "\"" << std::endl;
    
    close(fds[0]);
}

// 测试半关闭
void test_half_close() {
    std::cout << "\n===== 测试半关闭 =====" << std::endl;
    
    int fds[2];
    socketpair(AF_UNIX, SOCK_STREAM, 0, fds);
    
    wdf::TcpConnection conn(fds[0]);
    
    // 半关闭（关闭写端）
    conn.shutdown();
    
    // 对端应收到EOF
    char buf[10];
    int n = read(fds[1], buf, sizeof(buf));
    std::cout << "对端读取到: " << n << "字节（0表示EOF）" << std::endl;
    
    // 仍可接收数据
    write(fds[1], "data", 4);
    std::cout << "半关闭后收到: " << conn.receive() << std::endl;
    
    close(fds[0]);
    close(fds[1]);
}

int main() {
    test_basic_communication();
    test_connection_close();
    test_half_close();
    
    std::cout << "\n===== 测试完成 =====" << std::endl;
    return 0;
}
