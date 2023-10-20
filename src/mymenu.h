/**
 * @file menu.h
 * @brief 菜单类的头文件
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

#include "order.h"

/**
 * @brief 菜单类
 */
class Menu {
public:
    /**
     * @brief 存储输入的命令的类型，方便定位到指定的操作函数
     *  Create_Database，创建数据库
     *  Drop_Database，销毁数据库
     *  Use，切换数据库
     *  Create_Table，创建表
     *  Drop_Table，删除表
     *  Select，查询表
     *  Delete，删除表中的记录
     *  Insert，在表中插入数据
     *  Update，更新表中数据
     *  Unknown，未知，表示命令可能出错
     */
    enum Command_Type {
        Create_Database = 0,
        Drop_Database,
        Use,
        Create_Table,
        Drop_Table,
        Select,
        Delete,
        Insert,
        Update,
        Unknown
    };

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
     */
    void run();

private:
    /**
     * @brief 让用户输入命令，并且存储在类内command字符串对象中
     * @return 返回一个bool值来表示用户是否输入了结束的命令，程序需要退出
     */
    bool _command_input();

private:
    /**
     * @brief 维护一个命令管理Order对象，以后的命令都是由这里进行操作
     */
    Order order;
};

#endif
