#include "../InetAddress.hpp"
#include <cassert>
#include <iostream>

using std::cout;
using std::endl;

void test_constructor_with_ip_port() {
    // 测试带IP和端口的构造函数
    wdf::InetAddress addr1(8080, "127.0.0.1");
    assert(addr1.port() == 8080);
    assert(addr1.ip() == "127.0.0.1");
    
    // 测试默认IP
    wdf::InetAddress addr2(9090);
    assert(addr2.port() == 9090);
    assert(addr2.ip() == "0.0.0.0");
    
    // 测试无效IP
    wdf::InetAddress addr3(1234, "invalid.ip.address");
    // 无效IP应该被转换为0.0.0.0或其他默认值
    assert(addr3.ip() == "0.0.0.0");
}

void test_constructor_with_sockaddr_in() {
    // 测试从sockaddr_in构造
    struct sockaddr_in sa;
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(12345);
    inet_pton(AF_INET, "192.168.1.100", &sa.sin_addr);
    
    wdf::InetAddress addr(sa);
    assert(addr.port() == 12345);
    assert(addr.ip() == "192.168.1.100");
}

void test_ip_conversion() {
    // 测试IP地址转换
    wdf::InetAddress addr1(80, "8.8.8.8");
    assert(addr1.ip() == "8.8.8.8");

    wdf::InetAddress addr2(80, "255.255.255.255");
    assert(addr2.ip() == "255.255.255.255");

    wdf::InetAddress addr3(80, "0.0.0.0");
    assert(addr3.ip() == "0.0.0.0");
}

void test_port_conversion() {
    // 测试端口转换
    wdf::InetAddress addr1(0, "127.0.0.1");  // 最小端口
    assert(addr1.port() == 0);

    wdf::InetAddress addr2(65535, "127.0.0.1");  // 最大端口
    assert(addr2.port() == 65535);

    wdf::InetAddress addr3(443, "127.0.0.1");  // 常见端口
    assert(addr3.port() == 443);
}

void test_getAddrPtr() {
    // 测试获取底层sockaddr_in指针
    wdf::InetAddress addr(8080, "10.0.0.1");
    const struct sockaddr_in* sa = addr.getAddrPtr();

    assert(ntohs(sa->sin_port) == 8080);

    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(sa->sin_addr), ip, INET_ADDRSTRLEN);
    assert(std::string(ip) == "10.0.0.1");
    assert(sa->sin_family == AF_INET);
}

int main()
{
    test_constructor_with_ip_port();
    cout << endl;
    test_constructor_with_sockaddr_in();
    cout << endl;
    test_ip_conversion();
    cout << endl;
    test_port_conversion();
    cout << endl;
    test_getAddrPtr();

    cout << "All InetAddress tests passed!" << endl;
    return 0;
}

