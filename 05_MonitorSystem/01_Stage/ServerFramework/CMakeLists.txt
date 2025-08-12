cmake_minimum_required(VERSION 3.10)
project(ServerFramework)

set(CMAKE_CXX_STANDARD 17)

find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)

# 主服务器可执行文件
add_executable(server
    TestServer.cc
    MonitorServer.cc
    BusinessServer.cc
    MyLogger.cc
    Acceptor.cc
    Configuration.cc
    EventLoop.cc
    InetAddress.cc
    Socket.cc
    SocketIO.cc
    TcpConnection.cc
    Thread.cc
    Threadpool.cc
)
target_link_libraries(server PRIVATE spdlog::spdlog fmt::fmt pthread)

# 测试可执行文件
add_executable(test
    test.cc
    MyLogger.cc
)
target_link_libraries(test PRIVATE spdlog::spdlog fmt::fmt pthread)
