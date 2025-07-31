#ifndef SOCKET_HPP
#define SOCKET_HPP

namespace wdf
{

// 利用 RAII 的特性管理套接字的资源
class Socket
{
public:
    
    Socket();                   
    
    
    Socket(int);                
    
    // 释放套接字创建的在内核对象的端点对象资源
    ~Socket();


    int fd() { return m_fd; }  
    
   
    void shutdownWrite();       
                               

private:
    
    int m_fd;                  

};

}

#endif

