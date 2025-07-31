#include "TcpConnection.hpp"
#include "Acceptor.hpp"
#include <iostream>

using std::cout;
using std::endl;
using wdf::Acceptor;
using wdf::TcpConnection;

int main()
{
    Acceptor acceptor(8000);
    acceptor.ready();
    
    TcpConnection con(acceptor.accept());
    cout << con.toString() << endl;

    string msg = con.receive();
    cout << "recv msg: " << msg <<  endl;

    con.send(msg);

    return 0;
}

