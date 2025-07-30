# 编译器设置
# 	定义 C++ 编译器为 g++	                                                              
# 	编译选项: -Wall: 启用所有警告; -std=c++11: 使用 C++11 标准; -pthread: 启用多线程支持
# 	链接选项: -pthread: 链接多线程库	                                                  
CXX := g++										
CXXFLAGS := -Wall -std=c++11 -pthread			
LDFLAGS := -pthread								

# 目标可执行文件 
# 	定义最终要生成的可执行文件列表
TARGETS := tcpServer tcpClient					

# 源文件
# 	定义源文件列表（虽然这里没直接使用，但可以用于扩展）
SOURCES := 01_tcpServer.cc 02_tcpClient.cc		

# 默认目标 
# 	默认目标，当直接运行 make 时会执行; 依赖 $(TARGETS)，即会先构建 tcpServer 和 tcpClient
all: $(TARGETS)	

# 编译服务端
# 	目标 tcpServer 依赖于 01_tcpServer.cc  
# 	$(CXX): 使用 g++ 编译器                
# 	$(CXXFLAGS): 添加编译选项              
# 	-o $@: 输出文件名为目标名（tcpServer） 
# 	$<: 第一个依赖文件（01_tcpServer.cc）  
# 	$(LDFLAGS): 添加链接选项               
tcpServer: 01_tcpServer.cc						
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)		
												
# 编译客户端
tcpClient: 02_tcpClient.cc
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

# 清理生成的文件
clean:
	rm -f $(TARGETS)

# 伪目标声明
# 	声明 all 和 clean 为伪目标; 防止当目录下有同名文件时 Make 误判
.PHONY: all clean							

# $@: 当前规则的目标文件名
# $<: 当前规则的第一个依赖文件名
# $^: 当前规则的所有依赖文件列表
