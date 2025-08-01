#include "../SocketIO.hpp"
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <csignal>

using std::cout;
using std::endl;



void test_readline_multiline() {
    cout << "\n===== 测试多行readline =====" << endl;

    int fds[2];
    socketpair(AF_UNIX, SOCK_STREAM, 0, fds);

    wdf::SocketIO io(fds[0]);

    // 发送两行数据（注意第二个\n后没有数据）
    const char *data = "first line\nsecond line\n";
    write(fds[1], data, strlen(data));

    // 测试读取第一行
    char buf1[100] = {};
    int len1 = io.readline(buf1, sizeof(buf1));
    cout << "Line 1 (" << len1 << " bytes): " << buf1 << endl;

    // 测试读取第二行
    char buf2[100] = {};
    int len2 = io.readline(buf2, sizeof(buf2));
    cout << "Line 2 (" << len2 << " bytes): " << buf2 << endl;

    // 测试读取空数据（应返回0）
    char buf3[100] = {};
    int len3 = io.readline(buf3, sizeof(buf3));
    cout << "Line 3 (" << len3 << " bytes)" << endl;

    close(fds[0]);
    close(fds[1]);
}


int main() {
    // 忽略SIGPIPE信号
    signal(SIGPIPE, SIG_IGN);

    
    test_readline_multiline();
    
    std::cout << "\n===== 测试完成 =====" << std::endl;
    return 0;
}
