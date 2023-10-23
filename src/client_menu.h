/**
 * @file client_menu.h
 * @brief 客户端菜单类的头文件
 * @author lzx0626 (2065666169@qq.com)
 * @version 1.0
 * @date 2023-10-19
 *
 * @copyright Copyright (c) 2023  电子科技大学
 *
 */

#ifndef _MENU_H_
#define _MENU_H_

#include <cstring>
#include <exception>
#include <iostream>
#include <string>

#include "server_order.h"

/**
 * @brief 菜单类
 */
class Menu {
public:
    /**
     * @brief 默认构造函数
     */
    Menu() = default;

    /**
     * @brief 默认析构函数
     */
    ~Menu() = default;

public:
    /**
     * @brief 菜单创建之后就会执行的函数
     * @return std::string，返回输入之后经过适当处理之后的字符串
     */
    std::string run();

private:
    /**
     * @brief 维护一个执行run命令的次数，我们的客户端只有在第一次的时候才能显示所有的信息
     */
    int count = 0;
};

#endif
