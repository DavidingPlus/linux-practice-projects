/**
 * @file client.cpp
 * @brief 客户端的主程序
 * @author lzx0626 (2065666169@qq.com)
 * @version 1.0
 * @date 2023-10-19
 *
 * @copyright Copyright (c) 2023  电子科技大学
 *
 */

#include <arpa/inet.h>
#include <unistd.h>

#include <iostream>
#include <string>

#include "client_menu.h"

int main(int argc, char* const argv[]) {
    // 判断命令行参数
    if (argc < 3) {
        std::cout << "usage: " << argv[0] << " <ip> <address>" << std::endl;
        return -1;
    }

    std::string server_ip = std::string(argv[1]);
    unsigned short server_port = atoi(argv[2]);

    // 实例化菜单对象
    Menu menu;

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

    std::cout << "服务器连接成功!\n"
              << std::endl;

    // 3.开始通信
    char read_buf[BUFSIZ] = {0};
    while (1) {
        std::string send_commamd = menu.run();
        // std::cout << send_commamd << std::endl;

        // 发送命令
        send(connect_fd, send_commamd.c_str(), send_commamd.size(), 0);

        // 需要接收服务端发送回来的反馈
        bzero(read_buf, BUFSIZ);  // 这里千万忘了不要忘了清空，否则上一条是长命令，后面一条是短命令的话会导致长命令的后半段不能被清除掉
        int len = recv(connect_fd, read_buf, BUFSIZ - 1, 0);
        if (-1 == len) {
            perror("recv");
            return -1;
        }
        if (0 == len) {
            std::cout << "服务器关闭了..." << std::endl;
            break;
        } else if (len > 0) {
            std::cout << std::endl
                      << read_buf;
            // 判断得简单粗暴一点，如果是退出信息，服务端前五个字符是 "Thanks"
            // 注意，中文字符在char数组当中一个中文字符没办法用一个字节表示，所以我们不知道中文字符占了几个字节，所以我加上了英文前缀!
            if ("Thanks" == std::string(read_buf).substr(0, 6))
                break;

            std::cout << std::endl;
        }
    }

    // 4.关闭
    close(connect_fd);

    return 0;
}
