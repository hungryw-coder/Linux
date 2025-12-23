#include "MonitorServer.hpp"
#include "Configuration.hpp"
#include "MyLogger.hpp"

#include <iostream>

using std::cout;
using std::endl;
using wdf::Configuration;


int main()
{
    wdf::MyLogger::getInstance().init();
    Configuration::getInstance().loadConfig("./server.conf");
    Configuration::getInstance().printAll();
    size_t threadNum = Configuration::getInstance().getIntValue("thread_num");
    size_t taskNum = Configuration::getInstance().getIntValue("task_num");
    in_port_t port = Configuration::getInstance().getIntValue("port");
    MonitorServer server(threadNum, taskNum, port);
    
    server.start();
    return 0;
}

