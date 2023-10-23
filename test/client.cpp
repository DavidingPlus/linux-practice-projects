/**
 * @file client.cpp
 * @brief 客户端主程序
 * @author lzx0626 (2065666169@qq.com)
 * @version 1.0
 * @date 2023-10-19
 *
 * @copyright Copyright (c) 2023  电子科技大学
 *
 */

#include <arpa/inet.h>

#include <iostream>
#include <string>

#include "mymenu.h"

int main(int argc, char* const argv[]) {
    // 判断命令行参数
    if (argc < 3) {
        std::cout << "usage: " << argv[0] << " <ip> <address>" << std::endl;
        return -1;
    }

    std::string server_ip = std::string(argv[1]);
    unsigned short server_port = (unsigned short)atoi(argv[2]);

    // 1.创建socket套接字
    int connect_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (-1 == connect_fd) {
        perror("socket");
        return -1;
    }

    // 2.连接
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

    std::cout << "服务器连接成功!" << std::endl;

    // 3.开始通信
    // TODO

    // 4.关闭
    close(connect_fd);

    return 0;
}
