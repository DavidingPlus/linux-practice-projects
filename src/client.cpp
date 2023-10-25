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

#include <cstring>
#include <iostream>
#include <string>
using namespace std;
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

#define Max_Buffer_Size 1024

static const string prefix_path = "../copy/";

int main(int argc, char* const argv[]) {
    // 判断命令行参数
    if (argc < 3) {
        printf("usage: %s <ip>\n", argv[0]);
        return -1;
    }

    string server_ip = string(argv[1]);
    unsigned short server_port = 8080;

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

    cout << "连接成功,开始通信!" << endl;

    // 3.通信逻辑
    // 输入需要下载的文件的名字
    cout << "请输入您需要下载的文件名称: ";
    string file_name;
    cin >> file_name;

    string file_path = prefix_path + file_name;

    // 向服务器发送这个名字
    send(connect_fd, file_name.c_str(), file_name.size(), 0);

    // 发送了请求的文件之后需要接收返回值，只有返回值正确才能继续
    char read_buf[Max_Buffer_Size] = {0};
    int len = recv(connect_fd, read_buf, sizeof(read_buf) - 1, 0);
    if (-1 == len) {
        perror("recv");
        return -1;
    }

    // 创建一个新文件用于存储接收到的文件
    int fd = open(file_path.c_str(), O_RDWR | O_CREAT, 0755);
    if (-1 == fd) {
        perror("open");
        return -1;
    }

    // 接受数据
    while (1) {
        bzero(read_buf, sizeof(read_buf));
        int len = recv(connect_fd, read_buf, sizeof(read_buf) - 1, 0);
        if (-1 == len) {
            perror("recv");
            return -1;
        }
        if (0 == len) {  // 服务端那边发送完毕就会断开连接
            cout << "文件传输完毕!" << endl;
            break;
        }

        write(fd, read_buf, len);  // 这里最好用len好一点，因为数据里面有没有'\0'我们不知道，用strlen就可能会出问题
    }

    // 4.关闭
    close(connect_fd);

    return 0;
}
