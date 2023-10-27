# g++ ../test/server.cpp -I ../include/ -l mysrc -L ../res/ -std=c++20 -fsanitize=address

# 定义目标
target_client = client
target_server = server

# 定义主程序源文件
src_server = ../test/server.cpp
src_client = ../test/client.cpp

# 定义头文件搜索路径
include_path = ../include/

# 定义静态库名称
lib_name = mysrc

#定义静态库搜索路径
lib_path = ../res/

# 定义g++
CXX = g++

# 定义c++版本
version = -std=c++20

# 定义内存消毒器
memory = -fsanitize=address

# 定义目标文件为两个，这样可以编译出两个目标可执行文件
all : $(target_client) $(target_server)

$(target_server) : $(src_server)
	$(CXX) $(src_server) -I $(include_path) -l $(lib_name) -L $(lib_path) -o $(target_server) $(version) $(memory)

$(target_client) : $(src_client)
	$(CXX) $(src_client) -I $(include_path) -l $(lib_name) -L $(lib_path) -o $(target_client) $(version) $(memory)

.PHONY:clean
clean:
	rm $(target_server) $(target_client)
