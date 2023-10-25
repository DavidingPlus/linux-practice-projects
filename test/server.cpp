/**
 * @file server.cpp
 * @brief 服务端的主程序
 * @author lzx0626 (2065666169@qq.com)
 * @version 1.0
 * @date 2023-09-20
 *
 * @copyright Copyright (c) 2023  电子科技大学
 *
 */

#include <iostream>
#include <set>
using namespace std;
#include <arpa/inet.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/epoll.h>
#include <unistd.h>

#include "client_info.h"

const char* pre_key = "pT6)yO0)zA3?qC6+wD9+qD1?";

#define Max_Client_Size 1000
#define Max_Buffer_Size 1024
#define Client_Begin_FD 5  // 监听文件描述符3，epoll文件描述符4，所以从5开始

// 设置文件描述符为非阻塞
void set_nonblocking(int connect_fd);

int main() {
    // 存放客户端连接的IP和端口
    class Client_Info cli_infos[Max_Client_Size];
    // 存储服务器当中的人员信息
    // 为什么用set呢？因为方便通过值进行删除
    set<string> cli_names;

    // 1.创建socket套接字
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == listen_fd) {
        perror("socket");
        return -1;
    }

    // 设置端口复用
    int _optval = 1;
    int ret = setsockopt(listen_fd, SOL_SOCKET, SO_REUSEPORT, &_optval, sizeof(_optval));
    if (-1 == ret) {
        perror("setsockopt");
        return -1;
    }

    // 2.绑定IP和端口
    struct sockaddr_in server_addr;
    // 地址族
    server_addr.sin_family = AF_INET;
    // IP
    server_addr.sin_addr.s_addr = INADDR_ANY;
    // 端口
    server_addr.sin_port = htons(8080);

    ret = bind(listen_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (-1 == ret) {
        perror("bind");
        return -1;
    }

    // 3.监听端口
    ret = listen(listen_fd, 8);
    if (-1 == ret) {
        perror("listen");
        return -1;
    }

    cout << "服务器初始化成功!" << endl;

    // 4.用epoll技术实现接受客户端和进行通信
    // 创建epoll示例
    int epoll_fd = epoll_create(1);
    if (-1 == epoll_fd) {
        perror("epoll_create");
        return -1;
    }

    // 将监听套接字添加进入检测中
    struct epoll_event listen_event;
    listen_event.events = EPOLLIN;     // 检测读
    listen_event.data.fd = listen_fd;  // 文件描述符

    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &listen_event);

    int maxevents = Max_Client_Size;
    // 开始检测
    while (1) {
        // 这个结构体数组存放了检测到的文件描述符的信息，保存在这里面
        // 内核中是把双链表中的数据写入到这里
        struct epoll_event ret_events[maxevents];

        // 返回值是表示有多少个被检测到了；第三个参数可以一般放数组的最大容量
        int ret = epoll_wait(epoll_fd, ret_events, maxevents, -1);
        if (-1 == ret) {
            perror("epoll_wait");
            return -1;
        }

        // 检测到了，开始处理
        for (int i = 0; i < ret; ++i) {
            if (ret_events[i].data.fd == listen_fd) {
                // 表示有新客户端连接
                struct sockaddr_in client_addr;
                socklen_t client_addr_len = sizeof(client_addr);

                int connect_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &client_addr_len);
                if (-1 == connect_fd) {
                    perror("accept");
                    return -1;
                }

                // 设置read非阻塞
                set_nonblocking(connect_fd);

                // 将客户端信息存入结构体数组，下标采用connect_fd
                inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, cli_infos[connect_fd].client_ip, sizeof(cli_infos[connect_fd].client_ip));
                cli_infos[connect_fd].client_port = ntohs(client_addr.sin_port);

                // 服务端打印连接信息
                printf("client (ip : %s , port : %d) has connected...\n", cli_infos[connect_fd].client_ip, cli_infos[connect_fd].client_port);

                // 用监听文件描述符一样，添加到检测中
                struct epoll_event connect_event;
                connect_event.data.fd = connect_fd;
                connect_event.events = EPOLLIN;

                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, connect_fd, &connect_event);
            }
            // 服务端收到数据，把数据原封不动的发送给所有的客户端
            else {
                // 读
                char read_buf[Max_Buffer_Size] = {0};
                int len = recv(ret_events[i].data.fd, read_buf, sizeof(read_buf) - 1, 0);
                if (-1 == len) {
                    perror("recv");
                    exit(-1);
                }

                if (len > 0) {
                    // 我们在客户端的时候规定客户端在连接成功之后，就会立即向服务端发送用户名的消息
                    string data = read_buf;
                    int pos = data.find(':');
                    // 新客户端连接成功之后立即发送的用户名消息
                    if (string::npos != pos && pre_key == data.substr(0, pos)) {
                        // 维护目前服务器当中的用户人数和信息
                        char send_buf[Max_Buffer_Size] = {0};
                        sprintf(send_buf, "(服务器当中目前有%ld人", cli_names.size());
                        string send_buf_string(send_buf);
                        if (0 == cli_names.size())
                            send_buf_string += ")\n";
                        else {
                            send_buf_string += ": ";

                            for (auto iter = cli_names.begin(); iter != cli_names.end(); ++iter) {
                                send_buf_string += *iter;

                                auto tmp = iter;
                                send_buf_string += (++tmp == cli_names.end()) ? ")\n" : ",";
                            }
                        }
                        // 发送到本客户端
                        send(ret_events[i].data.fd, send_buf_string.c_str(), send_buf_string.size(), 0);

                        // 修改客户端信息的用户名
                        cli_infos[ret_events[i].data.fd].user_name = data.substr(pos + 1, data.size());

                        // 然后将新进来的人加入名字数组
                        cli_names.insert(cli_infos[ret_events[i].data.fd].user_name);

                        // 由于建立连接和发送用户名是连续的，所以我们在这里广播进来的消息
                        sprintf(send_buf, "(%s来了.)\n",
                                cli_infos[ret_events[i].data.fd].user_name.c_str());

                        // 这里不给自己发送
                        for (int j = Client_Begin_FD; j != ret_events[i].data.fd && -1 != cli_infos[j].client_port; ++j)
                            send(j, send_buf, strlen(send_buf), 0);
                    }
                    // 正常通信过程
                    else {
                        printf("client (ip : %s , port : %d) send: %s",
                               cli_infos[ret_events[i].data.fd].client_ip,
                               cli_infos[ret_events[i].data.fd].client_port,
                               read_buf);

                        // 读到正确消息之后就进行回写
                        char send_buf[Max_Buffer_Size + cli_infos[ret_events[i].data.fd].user_name.size() + 2] = {0};
                        sprintf(send_buf, "(%s): %s",
                                cli_infos[ret_events[i].data.fd].user_name.c_str(), read_buf);

                        // 给所有的客户端广播
                        for (int j = Client_Begin_FD; -1 != cli_infos[j].client_port; ++j)
                            send(j, send_buf, strlen(send_buf), 0);
                    }
                }
                // 写端，客户端关闭连接
                else if (0 == len) {
                    // 服务端打印信息
                    printf("client (ip : %s , port : %d) has closed...\n",
                           cli_infos[ret_events[i].data.fd].client_ip,
                           cli_infos[ret_events[i].data.fd].client_port);

                    char send_buf[Max_Buffer_Size] = {0};

                    sprintf(send_buf, "(%s离开了.)\n",
                            cli_infos[ret_events[i].data.fd].user_name.c_str());

                    // 向所有人广播
                    for (int j = Client_Begin_FD; -1 != cli_infos[j].client_port; ++j)
                        send(j, send_buf, strlen(send_buf), 0);

                    // 将客户端信息的set对应删除
                    cli_names.erase(cli_infos[ret_events[i].data.fd].user_name);
                    // 从检测事件中删除他
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, ret_events[i].data.fd, nullptr);
                    // 关闭文件描述符
                    close(ret_events[i].data.fd);
                }
            }
        }
    }

    // 5.关闭连接
    close(epoll_fd);
    close(listen_fd);

    return 0;
}

void set_nonblocking(int connect_fd) {
    int flag = fcntl(connect_fd, F_GETFL);
    flag |= O_NONBLOCK;
    fcntl(connect_fd, F_SETFL, flag);
}
