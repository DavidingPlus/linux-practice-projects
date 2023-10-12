cmake_minimum_required(VERSION 3.12)

# 设置cmake编译器版本，这两条命令要放在最上面命令的下面，其他命令的前面，否则无法生效
set (CMAKE_C_COMPILER "/usr/bin/gcc")
set (CMAKE_CXX_COMPILER "/usr/bin/g++")

project(MyProject)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 11)

# 添加可执行文件 client
add_executable(client
    test/client.cpp
)

# 添加可执行文件 server
add_executable(server
    test/server.cpp
)

# 指定头文件的搜索路径，要放在前面两个的后面，因为是根据可执行文件指定的
target_include_directories(client PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_include_directories(server PRIVATE ${CMAKE_SOURCE_DIR}/src)
