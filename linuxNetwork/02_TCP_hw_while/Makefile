# 编译器设置
CXX      := g++                   # C++ 编译器
CXXFLAGS := -Wall -std=c++11      # 编译选项
LDFLAGS  := -pthread              # 链接选项

# 构建目标
TARGETS  := server client   # 最终可执行文件
SOURCES  := server.cc client.cc   # 源文件

# 构建规则
all: $(TARGETS)                   # 默认目标

# 服务端构建
tcpServer: server.cc
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

# 客户端构建
tcpClient: client.cc
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

# 清理
clean:
	rm -f $(TARGETS)

.PHONY: all clean                 # 伪目标声明
