/**
 * @file server.cpp
 * @brief 服务端的主程序
 * @author lzx0626 (2065666169@qq.com)
 * @version 1.0
 * @date 2023-10-12
 *
 * @copyright Copyright (c) 2023  电子科技大学
 *
 */

#include <cstring>
#include <iostream>
using namespace std;
#include <arpa/inet.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>

#define Max_Buffer_Size 1024

// 存储文件的相对路径
static const string prefix_path = "../resources/";

// 子线程的业务函数
void* Work_Callback(void* args);

int main() {
    // 先用单进程实现一个简单的，再考虑后续的逻辑

    // 1.创建socket套接字
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == listen_fd) {
        perror("socket");
        return -1;
    }

    // 设置端口复用
    int optval = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));

    // 2.给服务端绑定端口
    struct sockaddr_in server_addr;
    // 地址族
    server_addr.sin_family = AF_INET;
    // IP地址
    server_addr.sin_addr.s_addr = INADDR_ANY;
    // 端口，我给定9999
    server_addr.sin_port = htons(9999);

    int ret = bind(listen_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (-1 == ret) {
        perror("bind");
        return -1;
    }

    cout << "服务器启动成功,开始监听!" << endl;

    // 3.开始监听
    ret = listen(listen_fd, 8);
    if (-1 == ret) {
        perror("listen");
        return -1;
    }

    // 4.建立连接
    // 这里建立了连接之后，主线程继续监听，创建一个子线程去处理业务逻辑
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);

        int connect_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &client_addr_len);
        if (-1 == connect_fd) {
            perror("accept");
            return -1;
        }

        // 创建子线程
        pthread_t tid;
        pthread_create(&tid, nullptr, Work_Callback, &connect_fd);
        pthread_detach(tid);  // 设置线程分离，自动回收
    }

    // 6.关闭

    close(listen_fd);

    return 0;
}

void* Work_Callback(void* args) {
    int connect_fd = *(int*)args;

    // 5.通信逻辑
    // 读取客户端请求的文件名字
    char file_name[Max_Buffer_Size] = {0};
    int len = recv(connect_fd, file_name, sizeof(file_name - 1), 0);
    if (-1 == len) {
        perror("recv");
        exit(-1);
    }

    // 去存储文件的路径去查询对应的文件
    string file_path = prefix_path + string(file_name);

    // 判断文件是否存在
    if (0 != access(file_path.c_str(), F_OK)) {
        // TODO
    } else {
        // 我们将一个文件的内容视作二进制流，用read去读取，一个字节一个字节的读取，然后打印出来

        // 打开源文件
        int fd = open(file_path.c_str(), O_RDONLY);
        if (-1 == fd) {
            perror("open");
            exit(-1);
        }

        char read_buf[Max_Buffer_Size] = {0};
        while (1) {
            // 读取数据
            bzero(read_buf, sizeof(read_buf));
            int len = read(fd, read_buf, sizeof(read_buf) - 1);
            if (-1 == len) {
                perror("read");
                exit(-1);
            }
            if (0 == len)  // 读取到文件末尾
                break;

            // 发送给服务端
            send(connect_fd, read_buf, len, 0);
        }
    }

    close(connect_fd);

    return nullptr;
}
