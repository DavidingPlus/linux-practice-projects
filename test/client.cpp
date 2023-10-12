/**
 * @file client.cpp
 * @brief 客户端的主程序
 * @author lzx0626 (2065666169@qq.com)
 * @version 1.0
 * @date 2023-10-12
 *
 * @copyright Copyright (c) 2023  电子科技大学
 *
 */

#include <iostream>
#include <string>
using namespace std;
#include <arpa/inet.h>
#include <unistd.h>

int main(int argc, char* const argv[]) {
    // 判断命令行参数
    if (argc < 3) {
        printf("usage: %s <ip> <port>\n", argv[0]);
        return -1;
    }

    string server_ip = string(argv[1]);
    unsigned short server_port = atoi(argv[2]);

    // 1.创建socket套接字
    int connect_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == connect_fd) {
        perror("socket");
        return -1;
    }

    // 2.建立连接
    struct sockaddr_in server_addr;
    // 地址族
    server_addr.sin_family = AF_INET;
    // IP
    inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr.s_addr);
    // 端口
    server_addr.sin_port = htons(server_port);

    int ret = connect(connect_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (-1 == ret) {
        perror("connect");
        return -1;
    }

    // 3.通信逻辑
    // TODO

    // 4.关闭
    close(connect_fd);

    return 0;
}
