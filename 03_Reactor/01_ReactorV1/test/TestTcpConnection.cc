#include "TcpConnection.hpp"
#include "Acceptor.hpp"
#include <iostream>

using std::cout;
using std::cerr; 
using std::endl;
using wdf::Acceptor;
using wdf::TcpConnection;

int main()
{
    Acceptor acceptor(8000);
    acceptor.ready();
    
    int confd = acceptor.accept();
    if (confd < 0) {
        cerr << "acceptor.accept() failed: " << strerror(errno) << endl;
        return -1;
    }

    sleep(1);

    TcpConnection con(confd);
    cout << con.toString() << endl;

    string msg = con.receive();
    cout << "recv msg: " << msg <<  endl;

    con.send(msg);

    return 0;
}

