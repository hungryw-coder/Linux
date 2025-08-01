#include "../Acceptor.hpp"
#include <iostream>

using std::cout;
using std::endl;

void run_server(wdf::Acceptor & acceptor)
{
    cout << "服务器准备接受连接..." << endl;
    int client_fd = acceptor.accept();
    if (client_fd > 0) {
        cout << "接受新连接: fd=" << client_fd << endl;
        char buf[100];
        int bytes_recv = recv(client_fd, buf, sizeof(buf), 0);
        cout << "收到消息" << std::string(buf, bytes_recv) << endl;
        send(client_fd, "world", 5, 0);
        close(client_fd);
    }
}


int main()
{
    wdf::Acceptor acc(8000);
    acc.ready();
    
    run_server(acc);
    
    return 0;
}

