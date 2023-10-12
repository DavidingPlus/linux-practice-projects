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

#include <iostream>
using namespace std;
#include <arpa/inet.h>
#include <unistd.h>

int main() {
    // 先用单进程实现一个简单的，再考虑后续的逻辑

    // 1.创建socket套接字
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == listen_fd) {
        perror("socket");
        return -1;
    }

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

    // 3.开始监听
    ret = listen(listen_fd, 8);
    if (-1 == ret) {
        perror("listen");
        return -1;
    }

    // 4.建立连接
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    int connect_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &client_addr_len);
    if (-1 == connect_fd) {
        perror("accept");
        return -1;
    }

    // 5.通信逻辑
    // TODO

    // 6.关闭
    close(connect_fd);
    close(listen_fd);

    return 0;
}
