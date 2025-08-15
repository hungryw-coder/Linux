#ifndef INETADDRESS_HPP
#define INETADDRESS_HPP

#include <my_header.h>
#include <string>

using std::string;

namespace wdf
{

// TCP通信 -- 网络地址
class InetAddress
{
public:
    // 可以通过 端口+IP字符串 创建地址
    InetAddress(in_port_t port, const string & ip = "0.0.0.0");         // ip 默认 "0.0.0.0" 表示监听所有网卡（通用性强）
                                                                        // port 参数写 in_port_t 是 POSIX 标准定义的类型，通常为 uint16_t（即 unsigned short），写 unsigned short 也行
    // 直接接受系统原生结构体
    InetAddress(const struct sockaddr_in &);                            // 兼容底层网络 API（如 accept() 返回的地址）

    // 提供对 m_addr 的安全访问，避免外部手动解析 sockaddr_in
    string ip() const;
    in_port_t port() const;

    // 供底层网络调用（如 bind()/connect()）直接使用
    const struct sockaddr_in * getAddrPtr() const { return &m_addr; }

private:
    // 存储标准的 IPv4 地址结构（包含 IP + 端口）
    struct sockaddr_in m_addr;  // IPv4
                                // 隐藏底层 sockaddr_in 的复杂性，对外提供更安全的接口（如 ip()/port() 方法）
                                // 避免外部直接修改导致数据不一致

};

}

#endif


