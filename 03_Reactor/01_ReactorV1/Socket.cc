#include "Socket.hpp"
#include <my_header.h>
#include <iostream>

using std::cout;
using std::endl;
using std::cerr;

namespace wdf 
{

Socket::Socket()
{
    m_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_fd < 0) {
        cerr << "Socket init failed: " << strerror(errno) << endl;
    }
}

Socket::Socket(int fd) 
: m_fd(fd)
{

}

Socket::~Socket()
{
    close(m_fd);
    cout << "Socket " << m_fd << "has closed." << endl;
}


void Socket::shutdownWrite()
{
    shutdown(m_fd, SHUT_WR);
}

}
