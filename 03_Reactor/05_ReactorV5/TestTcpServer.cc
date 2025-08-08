
#include "EchoTcpServer.hpp"

using wdf::EchoTcpserver;

int main()
{
    EchoTcpserver server(8080, "127.0.0.1", 4, 10);    
    server.start();

    return 0;
}

