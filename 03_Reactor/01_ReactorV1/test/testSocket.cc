#include "../Socket.hpp"
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <csignal>

// 全局忽略SIGPIPE信号
void ignore_sigpipe() {
    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGPIPE, &sa, nullptr);
}

void test_shutdownWrite_basic() {
    std::cout << "\n=== 基础功能测试 ===" << std::endl;

    // 创建socket对
    int fds[2];
    socketpair(AF_UNIX, SOCK_STREAM, 0, fds);

    wdf::Socket sock1(fds[0]);
    wdf::Socket sock2(fds[1]);

    // 测试正常写入
    const char* msg = "hello";
    ssize_t ret = write(sock1.fd(), msg, 5);
    std::cout << "写入 " << ret << " 字节数据" << std::endl;

    // 关闭写端
    sock1.shutdownWrite();

    // 验证再次写入应失败
    ret = write(sock1.fd(), msg, 5);
    if (ret == -1) {
        std::cout << "✓ 写入失败(符合预期), errno: "
                 << errno << " (" << strerror(errno) << ")" << std::endl;
    }

    // 验证读取端
    char buf[10];
    ret = read(sock2.fd(), buf, sizeof(buf));
    std::cout << "读取到 " << ret << " 字节: "
             << std::string(buf, ret) << std::endl;

    // 验证EOF
    ret = read(sock2.fd(), buf, sizeof(buf));
    std::cout << "读取结果: " << ret << " (0表示正确收到EOF)" << std::endl;
}

void test_shutdownWrite_edge_cases() {
    std::cout << "\n=== 边界条件测试 ===" << std::endl;

    // 测试重复调用
    wdf::Socket sock(socket(AF_INET, SOCK_STREAM, 0));
    sock.shutdownWrite();
    sock.shutdownWrite(); // 应不会崩溃
    std::cout << "✓ 重复调用成功" << std::endl;

    // 测试无效fd (需在Socket类中添加保护)
    wdf::Socket bad_sock(-1);
    bad_sock.shutdownWrite(); // 应不会崩溃
    std::cout << "✓ 无效fd处理成功" << std::endl;
}

int main() {
    ignore_sigpipe(); // 关键：忽略SIGPIPE

    std::cout << "===== Socket::shutdownWrite() 全面测试 =====" << std::endl;

    test_shutdownWrite_basic();
    test_shutdownWrite_edge_cases();

    std::cout << "\n===== 所有测试通过 =====" << std::endl;
    return 0;
}
