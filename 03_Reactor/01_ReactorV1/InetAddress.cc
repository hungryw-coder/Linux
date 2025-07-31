#include "InetAddress.hpp"

namespace wdf
{

// struct sockaddr_in {
//     sa_family_t    sin_family;   // 地址族（AF_INET 表示 IPv4）
//     in_port_t      sin_port;     // 16位端口号（需用 htons() 转换字节序）
//     struct in_addr sin_addr;     // 32位 IPv4 地址
//     char           sin_zero[8];  // 填充字段（通常置 0）
// };

// 简化版的 in_addr 结构
// struct in_addr {
//     in_addr_t s_addr;  // 32位 IPv4 地址（需用 inet_pton() 或 htonl() 处理）
// };

// const char *inet_ntop(                              // 将 二进制网络字节序（struct in_addr 或 struct in6_addr） 转换为 点分十进制字符串（如 "192.168.1.1"）
//                       int af,                       // 地址族：AF_INET（IPv4）或 AF_INET6（IPv6）
//                       const void *restrict src,     // 二进制地址（struct in_addr 或 struct in6_addr）
//                       char dst[restrict .size],     // 存储转换后的字符串（必须足够大）
//                       socklen_t size                // dst 缓冲区的大小（防止溢出）
//                       );                            // dst（非 NULL）成功，返回 dst 指针; NULL	失败（errno 会设置）

// int inet_pton(                               // 将 点分十进制字符串（如 "127.0.0.1"） 转换为 二进制网络字节序（struct in_addr 或 struct in6_addr）
//               int af,                        // 地址族：AF_INET（IPv4）或 AF_INET6（IPv6）  
//               const char *restrict src,      // 点分十进制字符串（如 "192.168.1.1"）
//               void *restrict dst             // 存储转换后的二进制地址（struct in_addr 或 struct in6_addr）
//               );                             // 1 代表转换成功；0 代表src 不是有效的 IP 地址；-1	代表 af 不是 AF_INET 或 AF_INET6

InetAddress::InetAddress(in_port_t port, const string & ip)
{
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sin_family = AF_INET;
    m_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &m_addr.sin_addr);
}

InetAddress::InetAddress(const struct sockaddr_in & addr)
: m_addr(addr)
{
    
}

string InetAddress::ip() const 
{   
    char buf[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &m_addr.sin_addr, buf, sizeof(buf));
    return string(buf);
}

unsigned short InetAddress::port() const 
{
    return ntohs(m_addr.sin_port);
}

}

