cmake_minimum_required(VERSION 3.12)

# 设置cmake编译器，这两条命令要放在最上面命令的下面，其他命令的前面，否则无法生效
set (CMAKE_C_COMPILER "/usr/bin/gcc-13")
set (CMAKE_CXX_COMPILER "/usr/bin/g++-13")

project(MyProject)

#设置CMake_Build_Type
set(CMAKE_BUILD_TYPE Debug)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 20)

# 添加源文件
set(SOURCE_FILES
    src/http_conn.cpp
    src/locker.cpp
    src/threadpool.cpp
    test/main.cpp
)

# 添加可执行文件 app，并指定源文件
add_executable(app ${SOURCE_FILES})

# 添加头文件搜索路径，以便找到头文件
target_include_directories(app PRIVATE src)  # 添加 src 目录

# 可选：如果您的项目需要链接其他库，可以使用 target_link_libraries 命令
# target_link_libraries(app PRIVATE some_library)

# 设置输出路径，将编译后的可执行文件放入 build/bin 文件夹
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/)
