/**
 * @file client.cpp
 * @brief 客户端的主程序
 * @author lzx0626 (2065666169@qq.com)
 * @version 1.0
 * @date 2023-09-18
 *
 * @copyright Copyright (c) 2023  电子科技大学
 *
 */

#include <cstring>
#include <iostream>
#include <string>
using namespace std;
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>

#include "main_menu.h"

#define Max_Buffer_Size 1024

const char* pre_key = "pT6)yO0)zA3?qC6+wD9+qD1?";

// 通信的逻辑函数
void communication(const string& name, int connect_fd);

// 线程的回调函数
void* Callback_Read(void* args);
void* Callback_Write(void* args);

// 全局存储服务端的IP地址
static string server_ip;
// 全局存储端口
static in_port_t server_port;

int main(int argc, char const* argv[]) {
    if (argc < 3) {
        printf("usage: %s <ip_address> <port>\n", argv[0]);
        return -1;
    }

    // 全局存储服务端的IP地址
    server_ip = argv[1];
    // 全局存储端口
    server_port = atoi(argv[2]);

    // 先尝试连接到服务端
    // 1.创建套接字
    int connect_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == connect_fd) {
        perror("socket");
        exit(-1);
    }

    // 2.建立连接
    struct sockaddr_in server_addr;
    // 地址族
    server_addr.sin_family = AF_INET;
    // 端口
    server_addr.sin_port = htons(server_port);
    // IP
    inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr.s_addr);

    int ret = connect(connect_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (-1 == ret) {
        perror("connect");
        exit(-1);
    }

    cout << "连接成功!" << endl;

    // 连接成功之后正式开始业务逻辑
    Menu m(connect_fd);
    m.run(communication);

    return 0;
}

void communication(const string& name, int connect_fd) {
    // 客户端在连接成功之后立马向服务端发送一个名字的信息，前缀pre_key和服务端提前规定好
    char send_buf[Max_Buffer_Size] = {0};
    sprintf(send_buf, "%s:%s", pre_key, name.c_str());
    send(connect_fd, send_buf, strlen(send_buf), 0);

    // 3.正式开始通信
    // 客户端和服务端不同，服务端是读取了数据，然后就发送给客户端，所以有先后顺序
    // 但是客户端等待写入数据也可能有其他用户输入数据然后收到数据，所以这里用两个子线程处理
    pthread_t read_tid, write_tid;
    // 创建线程
    pthread_create(&read_tid, nullptr, Callback_Read, &connect_fd);
    pthread_create(&write_tid, nullptr, Callback_Write, &connect_fd);
    // 设置线程分离
    pthread_detach(read_tid);
    pthread_detach(write_tid);

    // 退出主线程
    pthread_exit(nullptr);

    // 4.关闭连接
    close(connect_fd);
}

void* Callback_Read(void* args) {
    int connect_fd = *(int*)args;

    while (1) {
        // 读
        char read_buf[Max_Buffer_Size] = {0};
        int len = recv(connect_fd, read_buf, sizeof(read_buf) - 1, 0);
        if (-1 == len) {
            perror("recv");
            exit(-1);
        }

        if (len > 0)
            printf("%s", read_buf);
        else if (0 == len) {
            cout << "服务器关闭!" << endl;
            close(connect_fd);
            exit(-1);
        }
    }

    return nullptr;
}

void* Callback_Write(void* args) {
    // 这里需要进行类型转换
    int connect_fd = *(int*)args;

    while (1) {
        // 写
        char send_buf[Max_Buffer_Size] = {0};
        fgets(send_buf, sizeof(send_buf) - 1, stdin);
        // 退出功能，q或者退出
        if (0 == strcmp("q\n", send_buf) || 0 == strcmp("退出\n", send_buf)) {
            cout << "退出成功!" << endl;
            close(connect_fd);
            exit(0);
        }

        send(connect_fd, send_buf, strlen(send_buf), 0);
    }

    return nullptr;
}
