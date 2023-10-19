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
     * @brief 打开对应位置的文件，并且将里面的内容打印出来
     * @param  path，文件对应的目录，可能是绝对路径，也可能是相对路径
     */
    void _open_print(const std::string& path);

    /**
     * @brief 让用户输入命令，并且存储在类内command字符串对象中
     * @return 返回一个bool值来表示用户是否输入了结束的命令，程序需要退出
     */
    bool _command_input();

    /**
     * @brief 根据给定的命令找到对应的命令类型，在这个函数当中不考虑命令的具体合理性问题，这个交给另一个类去做，我们只是初步判断这个命令可能的类型
     * @param  command，命令字符串
     * @return Command_Type，命令枚举类型
     */
    Command_Type _get_type(const std::string& command);

    /**
     * @brief 和上面的函数配套使用，在确定是create和drop的前提下进一步确定是database还是table
     * @param  pos，第一个空格在原命令字符串当中的下标
     * @param  command，原命令字符串
     * @param  first，确认第一个参数是create还是drop，true代表是create，false代表是drop
     * @return Command_Type，命令枚举类型
     */
    Command_Type _get_database_table(size_t pos, const std::string& command, bool first);

private:
    /**
     * @brief 存储当前用户输入命令的字符串
     */
    std::string command;

    /**
     * @brief 与上面字符串命令对应的命令类型
     */
    Command_Type cmd_type = Unknown;
};

#endif
