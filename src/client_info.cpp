/**
 * @file Client_Info.cpp
 * @brief 存储客户端套接字数据的类的源文件
 * @author lzx0626 (2065666169@qq.com)
 * @version 1.0
 * @date 2023-09-20
 *
 * @copyright Copyright (c) 2023  电子科技大学
 *
 */

/**
 * @brief 以下是对类内函数的实现
 */

#include "client_info.h"

Client_Info::Client_Info() {
    _init();
};

Client_Info::Client_Info(const char* ip, const in_port_t& port, const std::string& user_name) {
    strcpy(this->client_ip, ip);
    this->client_port = port;
    this->user_name = user_name;
}

Client_Info& Client_Info::operator=(const Client_Info& other) {
    strcpy(this->client_ip, other.client_ip);
    this->client_port = other.client_port;

    return *this;
}

Client_Info::Client_Info(const Client_Info& other) {
    *this = other;
}

void Client_Info::_init() {
    bzero(this->client_ip, sizeof(this->client_ip));
    this->client_port = -1;
}
