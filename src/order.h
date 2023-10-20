/**
 * @file order.h
 * @brief 处理输入命令的类的头文件
 * @author lzx0626 (2065666169@qq.com)
 * @version 1.0
 * @date 2023-10-20
 *
 * @copyright Copyright (c) 2023  电子科技大学
 *
 */

#ifndef _ORDER_H_
#define _ORDER_H_

#include <sys/stat.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <string>
#include <vector>

/**
 * @brief 打开对应位置的文件，并且将里面的内容打印出来，定义成为extern，因为两个源文件都需要使用
 * @param  path，文件对应的目录，可能是绝对路径，也可能是相对路径
 */
extern void _open_print(const std::string& path);

class Order {
public:
    /**
     * @brief 存储输入的命令的类型，方便定位到指定的操作函数
     *  Show，展示命令的格式规范
     *  Quit，退出程序
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
        Show = 0,
        Quit,
        Create_Database,
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
    Order() = default;

    /**
     * @brief 默认析构函数
     */
    ~Order() = default;

public:
    /**
     * @brief 每次输入新的命令的时候，需要提前将命令的成员全部清空
     */
    void clear();

    /**
     * @brief 提供得到命令字符串的接口
     * @return std::string，返回命令字符串
     */
    std::string get_command();

    /**
     * @brief 设置命令字符串
     * @param  order，传入的命令字符串
     */
    void set_command(const std::string& order);

    /**
     * @brief 给外部提供一个run接口，表示拿到命令之后需要开始执行
     */
    void run();

private:
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

    //***************************下面就是具体业务的函数逻辑***************************

    /**
     * @brief 处理Show类型命令
     */
    void _deal_show();

    /**
     * @brief 处理Quit类型命令
     */
    void _deal_quit();

    /**
     * @brief 处理Create_Database类型命令
     */
    void _deal_create_database();

    /**
     * @brief 处理Unknown类型命令
     */
    void _deal_unknown();

private:
    /**
     * @brief 存储当前用户输入命令的字符串
     */
    std::string m_command;

    /**
     * @brief 与上面字符串命令对应的命令类型
     */
    Command_Type m_command_type = Unknown;

    /**
     * @brief 文件目录或者文件名当中不能出现的字符集合
     */
    static std::vector<char> banned_ch;

    /**
     * @brief 存放数据库的相对目录前缀
     */
    static std::string database_prefix;
};

#endif
