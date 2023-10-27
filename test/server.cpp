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

#include "server_order.h"

/**
 * @brief 定义ipv4地址的char*字符串最大长度
 */
#define Max_ipv4_len 16

/**
 * @brief 定义epoll能检测的最大事件个数max_events
 */
#define max_events 1000

/**
 * @brief 拿一个结构体来存储连接的客户端信息
 */
struct Client_Info {
    /**
     * @brief 构造函数，对成员进行初始化
     */
    Client_Info() { clear(); }

    /**
     * @brief 将类内部成员清零
     */
    void clear() {
        ip.clear();
        port = -1;
    }

    /**
     * @brief 客户端的IP
     */
    std::string ip;

    /**
     * @brief 客户端的端口，为了让没开的端口设置为-1，我这里用的类型是-1，当然正常使用的时候会隐式转换为unsigned short，没有区别
     */
    int port;
};

int main() {
    // 创建存储客户端信息的结构体
    struct Client_Info cli_infos[max_events + 10];  // 0 1 2文件描述符被占用，从3开始，用文件描述符当作下标，多开10个有备无患

    // 实例化Order对象
    Order order;

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
    server_addr.sin_port = htons(8080);

    ret = bind(listen_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (-1 == ret) {
        perror("bind");
        return -1;
    }

    std::cout << "server has successfully initialized." << std::endl;

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

                // 将客户端信息存入客户端信息数组当中，connect_fd作下标
                cli_infos[connect_fd].ip = std::string(client_ip);
                cli_infos[connect_fd].port = client_port;

                std::cout << "client (ip: " << client_ip << " , "
                          << "port: " << client_port << ") has connected." << std::endl;

                // 设置读取非阻塞，必须设置，虽然在这个示例当中没有太大影响
                // 但是IO多路复用技术是建立在非阻塞IO基础上的
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
                int connect_fd = ret_events[i].data.fd;
                std::string client_ip = cli_infos[connect_fd].ip;
                unsigned short client_port = cli_infos[connect_fd].port;

                // 接受客户端的命令
                char read_buf[BUFSIZ] = {0};
                int len = recv(connect_fd, read_buf, BUFSIZ - 1, 0);
                if (-1 == len) {
                    // 非阻塞有两种特殊情况返回-1，但是我们这里遇不到
                    // errno==EINTR，收到信号并从信号处理函数返回时，慢系统调用会返回并设置errno为EINTR，应该重新调用read。
                    // errno==EAGAIN，表示当前暂时没有数据可读，应该稍后读取。
                    perror("recv");
                    return -1;
                }
                if (0 == len) {  // 客户端关闭
                    // 从监听事件中删除
                    ret = epoll_ctl(epoll_fd, EPOLL_CTL_DEL, connect_fd, nullptr);
                    if (-1 == ret) {
                        perror("epoll_ctl");
                        return -1;
                    }

                    std::cout << "client (ip: " << client_ip << " , "
                              << "port: " << client_port << ") has closed." << std::endl;
                    // 关闭文件描述符
                    close(connect_fd);
                    break;
                } else if (len > 0) {
                    std::cout << "client (ip: " << client_ip << " , "
                              << "port: " << client_port << ") send: " << read_buf << std::endl;

                    // 处理该命令
                    order.set_command(std::string(read_buf));

                    // 我们设计order里面的标准输出全部重定向文件当中，这样我们可以非常方便的读取m_feedback，完事之后再设置回去
                    // 先把文件长度截断为0，然后从头开始写，可以下面这么做，我这里在open中加入O_TRUNC也可以
                    // truncate(std::string(Order::res_prefix + "feedback.txt").c_str(), 0);

                    int fd = open(std::string(Order::res_prefix + "feedback.txt").c_str(), O_RDWR | O_TRUNC);
                    if (-1 == fd) {
                        perror("open");
                        return -1;
                    }

                    int copy = dup(STDOUT_FILENO);  // 拷贝原先标准输出的文件描述符
                    dup2(fd, STDOUT_FILENO);        // dup2的第二个参数会指向第一个参数指向的文件描述符

                    order.run();

                    dup2(copy, STDOUT_FILENO);  // 重定向回去
                    close(fd);

                    // order读取feedback.txt中的反馈信息
                    order.read_feedback();

                    // 拿到Order类中存储的m_feedback字符串，发送回去
                    send(connect_fd, order.get_feedback().c_str(), order.get_feedback().size(), 0);
                }
            }
        }
    }

    // 6.关闭
    close(epoll_fd);
    close(listen_fd);

    return 0;
}
