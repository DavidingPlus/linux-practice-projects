/**
 * @file Menu.h
 * @brief 聊天室启动的终端界面的菜单，头文件
 * @author lzx0626 (2065666169@qq.com)
 * @version 1.0
 * @date 2023-09-18
 *
 * @copyright Copyright (c) 2023  电子科技大学
 *
 */

#ifndef _MENU_H_
#define _MENU_H_

#include <cstring>
#include <iostream>
#include <string>

#define Max_Buffer_Size 1024

/**
 * @brief 菜单类
 */
class Menu {
public:
    /**
     * @brief 默认构造函数，但是被我废除，必须指定文件描述符
     */
    Menu() = delete;

    /**
     * @brief 构造函数，我们需要知道客户端通信使用的文件描述符
     * @param  connect_fd，文件描述符
     */
    Menu(int connect_fd);

public:
    /**
     * @brief 定义一个回调函数类型
     */
    using call_back = void(const std::string& name, int connect_fd);

    /**
     * @brief 业务逻辑函数
     * @param call，回调函数
     */
    void run(call_back call);

private:
    /**
     * @brief 初始化
     * @param  connect_fd，文件描述符
     */
    void _init(int connect_fd);

    /**
     * @brief 处理匿名聊天，选项1
     * @param call，回调函数
     */
    void _anonymous_chat(call_back call);

    /**
     * @brief 处理登录，选项2
     * @param call，回调函数
     */
    void _sign_in(call_back call);

    /**
     * @brief 处理注册，选项3
     */
    void _sign_up();

    /**
     * @brief 处理退出，选项4
     */
    void __exit();

    /**
     * @brief 处理输入错误的情况
     */
    void _error();

    /**
     * @brief 将得到的用户名或者密码在文本中进行搜索匹配
     */
    bool _if_exists(const std::string& Username, const std::string& Password);

private:
    /**
     * @brief 存储用户名密码的文件路径，目前我的想法是用txt文本文件存储
     */
    std::string file_path;

    /**
     * @brief 存储用户的用户名，这个也会作为通信过程中的用户名
     */
    std::string username;

    /**
     * @brief 存储用户的密码
     */
    std::string password;

    /**
     * @brief 存储客户端使用的文件描述符
     */
    int connect_fd = -1;
};

#endif
