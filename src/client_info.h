/**
 * @file Client_Info.h
 * @brief 存储客户端套接字数据的类的头文件
 * @author lzx0626 (2065666169@qq.com)
 * @version 1.0
 * @date 2023-09-20
 *
 * @copyright Copyright (c) 2023  电子科技大学
 *
 */

#ifndef _CLIENT_INFO_H_
#define _CLIENT_INFO_H_

#include <arpa/inet.h>

#include <cstring>
#include <iostream>

#define Max_IPv4_Size 16

/**
 * @brief 存储客户端套接字数据的类
 */
class Client_Info {
public:
    /**
     * @brief 构造函数
     */
    Client_Info();

    /**
     * @brief 构造函数
     * @param  ip，IPv4地址
     * @param  port，端口号
     */
    Client_Info(const char* ip, const in_port_t& port, const std::string& user_name);

    /**
     * @brief 拷贝赋值函数
     * @param  other
     * @return Client_Info&，对象引用，返回自身
     */
    Client_Info& operator=(const Client_Info& other);

    /**
     * @brief 拷贝构造函数
     * @param  other
     */
    Client_Info(const Client_Info& other);

private:
    /**
     * @brief 初始化对象
     */
    void _init();

public:
    /**
     * @brief 存储IPv4地址
     */
    char client_ip[Max_IPv4_Size];

    /**
     * @brief 存储端口号
     */
    int client_port;

    /**
     * @brief 存储用户名
     */
    std::string user_name;
};

#endif
