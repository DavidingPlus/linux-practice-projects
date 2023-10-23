/**
 * @file server.cpp
 * @brief 服务端的主程序
 * @author lzx0626 (2065666169@qq.com)
 * @version 1.0
 * @date 2023-10-23
 *
 * @copyright Copyright (c) 2023  电子科技大学
 *
 */

#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

/**
 * @brief 定义ipv4地址的char*字符串最大程度
 */
#define Max_ipv4_len 16

int main() {
    // 先开单进程服务端测试一下

    // 1.创建socket套接字
    int listen_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (-1 == listen_fd) {
        perror("socket");
        return -1;
    }

    // 设置端口复用
    int optval = 1;
    int ret = setsockopt(listen_fd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
    if (-1 == ret) {
        perror("bind");
        return -1;
    }

    // 2.绑定IP和端口
    struct sockaddr_in server_addr;
    // 地址族
    server_addr.sin_family = AF_INET;
    // IP
    server_addr.sin_addr.s_addr = INADDR_ANY;
    // 端口
    server_addr.sin_port = htons(9999);

    ret = bind(listen_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (-1 == ret) {
        perror("bind");
        return -1;
    }

    std::cout << "server has initialized." << std::endl;

    //********************从这里开始，修改成为epoll架构********************

    // 3.开始监听
    ret = listen(listen_fd, 8);
    if (-1 == ret) {
        perror("listen");
        return -1;
    }

    // 创建epoll实例
    int epoll_fd = epoll_create(1);
    if (-1 == epoll_fd) {
        perror("epoll_create");
        return -1;
    }

    // 将listen_fd加入epoll监听事件中
    struct epoll_event listen_event;
    listen_event.data.fd = listen_fd;
    listen_event.events = EPOLLIN;

    ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &listen_event);
    if (-1 == ret) {
        perror("epoll_ctl");
        return -1;
    }

    // 定义最大事件数max_events
    int max_events = 1000;

    // 开始检测
    while (1) {
        struct epoll_event ret_events[max_events] = {0};
        int count = epoll_wait(epoll_fd, ret_events, max_events, -1);  //-1表示阻塞
        if (-1 == count) {
            perror("epoll_wait");
            return -1;
        }

        for (int i = 0; i < count; ++i) {
            // 新客户端加入
            if (listen_fd == ret_events[i].data.fd) {
                // 接受请求
                struct sockaddr_in client_addr;
                socklen_t client_addr_len = sizeof(client_addr);

                int connect_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &client_addr_len);
                if (-1 == connect_fd) {
                    perror("accept");
                    return -1;
                }

                // 获得客户端信息
                unsigned short client_port = ntohs(client_addr.sin_port);
                char client_ip[Max_ipv4_len] = {0};
                inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, client_ip, Max_ipv4_len);

                std::cout << "client (ip: " << client_ip << " , "
                          << "port: " << client_port << ") has connected." << std::endl;

                // 设置读取非阻塞，必须设置，虽然在这个示例当中没有太大影响
                // 但是IO多路技术是建立在非阻塞IO基础上的
                int flag = fcntl(connect_fd, F_GETFD);
                flag |= O_NONBLOCK;
                fcntl(connect_fd, F_SETFD, flag);

                // 将新客户端加入到检测事件
                struct epoll_event connect_event;
                connect_event.data.fd = connect_fd;
                connect_event.events = EPOLLIN;

                ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, connect_fd, &connect_event);
                if (-1 == ret) {
                    perror("epoll_ctl");
                    return -1;
                }
            }
            // 老客户端通信
            else {
                // TODO
            }
        }
    }

    // 6.关闭
    close(epoll_fd);
    close(listen_fd);

    return 0;
}
