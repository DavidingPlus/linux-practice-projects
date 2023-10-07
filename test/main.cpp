/**
 * @file main.cpp
 * @brief 主程序
 * @author 刘治学
 */
#include <arpa/inet.h>
#include <signal.h>
#include <sys/epoll.h>
#include <unistd.h>

#include <cstring>
#include <exception>
#include <iostream>

#include "http_conn.h"
#include "locker.h"
#include "threadpool.h"

#define MAX_CLIENT_NUM 65535  // 最大的用户(文件描述符个数)
#define MAX_EVENT_NUM 10000   // 同时监听的最大的事件数量

// 添加信号捕捉
void add_sigact(int sig, void (*handler)(int));

// 将文件描述符添加到epoll示例中
extern void add_fd(int epollfd, int fd, bool one_shot);

// 将文件描述符从epoll实例中删除
extern void remove_fd(int epollfd, int fd);

// 修改epoll中的文件描述符，传入事件
extern void modify_fd(int epollfd, int fd, int ev);

int main(int argc, char const *argv[]) {
    // 命令行参数判断--------------------------------
    if (argc <= 1) {
        printf("usage : %s  <port>\n", basename(argv[0]));  // basename可以获取基础的名称，去掉路径
        return -1;
    }

    // 获取端口号
    in_port_t port = atoi(argv[1]);

    // 对一些信号做一些处理---------------------------
    // 比如读端关闭，但是写端还未关闭并且想继续写数据，就会产生SIGPIPE信号，程序异常终止
    add_sigact(SIGPIPE, SIG_IGN);

    // 初始化线程池----------------------------------
    Thread_Pool<Http_Conn> *pool = new Thread_Pool<Http_Conn>;
    if (!pool)
        throw std::exception();

    // 创建一个数组用于保存所有的客户端信息--------------
    Http_Conn *users = new Http_Conn[MAX_CLIENT_NUM];

    // TCP通信过程-----------------------------------
    // 创建TCP套接字
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

    // 绑定端口
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    ret = bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (-1 == ret) {
        perror("bind");
        return -1;
    }

    // 监听
    ret = listen(listen_fd, 5);
    if (-1 == ret) {
        perror("listen");
        return -1;
    }

    // 创建epoll示例
    int epoll_fd = epoll_create(1);
    if (-1 == epoll_fd) {
        perror("epoll_create");
        return -1;
    }

    // 将监听文件描述符添加到epoll对象
    add_fd(epoll_fd, listen_fd, false);
    // 设置全局一份的epoll_fd
    Http_Conn::_epollfd = epoll_fd;

    // 创建存放检测结果的数组，开始检测
    epoll_event events[MAX_EVENT_NUM];

    while (1) {
        bzero(events, sizeof(events));
        int num = epoll_wait(epoll_fd, events, MAX_EVENT_NUM, -1);
        if (-1 == num) {
            // 有一种情况是被信号中断然后变为非阻塞，这时的错误号是EINTR
            if (EINTR == errno)
                continue;
            perror("epoll_wait");
            return -1;
        }

        // 循环遍历事件数组
        for (int i = 0; i < num; ++i) {
            int socket_fd = events[i].data.fd;
            // 客户端连接进来
            if (listen_fd == socket_fd) {
                struct sockaddr_in client_addr;
                socklen_t client_addr_len;

                int connect_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &client_addr_len);
                if (-1 == connect_fd) {
                    perror("accept");
                    return -1;
                }

                // 如果目前连接数满了，就释放连接
                if (Http_Conn::_user_count >= MAX_CLIENT_NUM) {
                    // 可以选择给客户端写一个信息，就是服务器正忙
                    close(connect_fd);
                    continue;
                }
                // 将新客户的数据初始化，放到数组中，我就用文件描述符作为索引
                users[connect_fd].__init__(connect_fd, client_addr);
            }  // 处理对方异常断开或者错误的事件
            else if (events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR))
                users[socket_fd].close_conn();
            // 检测到读事件发生
            else if (events[i].events & EPOLLIN) {  // 这个判断不加等号是因为为0就代表这一位为0，没检测到
                // 模拟Proactor模式
                if (users[socket_fd]._read())  // 一次性把所有数据读出来，直到没有更多数据可读，读成功则加入任务请求队列
                    pool->add_task(users[socket_fd]);
                else  // 读取数据失败
                    users[socket_fd].close_conn();
            }  // 检测到写事件发生
            else if (events[i].events & EPOLLOUT)
                if (!users[socket_fd]._write())  // 一次性把所有数据写完，失败则关闭
                    users[socket_fd].close_conn();
        }
    }

    // 关闭文件描述符，释放堆上的数据-----------------------
    close(epoll_fd);
    close(listen_fd);

    delete[] users;
    delete pool;

    return 0;
}

void add_sigact(int sig, void (*handler)(int)) {
    struct sigaction act;
    sigemptyset(&act.sa_mask);

    act.sa_flags = 0;  // 使用第一个 sa_handler

    act.sa_handler = handler;

    sigfillset(&act.sa_mask);  // 临时信号集，全部阻塞

    sigaction(sig, &act, nullptr);
}
