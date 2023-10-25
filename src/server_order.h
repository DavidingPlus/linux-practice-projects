/**
 * @file server_order.h
 * @brief 服务端处理输入命令的类的头文件
 * @author lzx0626 (2065666169@qq.com)
 * @version 1.0
 * @date 2023-10-20
 *
 * @copyright Copyright (c) 2023  电子科技大学
 *
 */

#ifndef _SERVER_ORDER_H_
#define _SERVER_ORDER_H_

#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <iostream>
#include <string>
#include <vector>

#include "server_table.h"
#include "tools.h"

class Order {
public:
    /**
     * @brief 存储输入的命令的类型，方便定位到指定的操作函数
     *  Show，展示命令的格式规范
     *  Tree，展示数据库的目录架构
     *  Quit，退出程序
     *  Clear，清空屏幕
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
        Tree,
        Quit,
        Clear,
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
     * @brief 设置命令字符串
     * @param  order，传入的命令字符串
     */
    void set_command(const std::string& order);

    /**
     * @brief 给外部提供一个run接口，表示拿到命令之后需要开始执行
     */
    void run();

    /**
     * @brief 得到反馈字符串
     * @return std::string
     */
    std::string get_feedback() const { return m_feedback; }

    /**
     * @brief 从feedback.txt文件中读取信息并且修改本类中的m_feedback字符串信息
     */
    void read_feedback();

private:
    /**
     * @brief 根据给定的命令找到对应的命令类型，在这个函数当中不考虑命令的具体合理性问题，这个交给另一个类去做，我们只是初步判断这个命令可能的类型
     * @param  command，命令字符串
     * @return Command_Type，命令枚举类型
     */
    Command_Type _get_type(const std::string& command);

    /**
     * @brief 和上面的函数配套使用，在确定是create和drop的前提下进一步确定是database还是table
     * @param  blank_pos，第一个空格在原命令字符串当中的下标
     * @param  command，原命令字符串
     * @param  first，确认第一个参数是create还是drop，true代表是create，false代表是drop
     * @return Command_Type，命令枚举类型
     */
    Command_Type _get_database_table(size_t blank_pos, const std::string& command, bool first);

    //------------------------------------------------------------

    // public:
    /**
     * @brief 定义线程回调函数的类型
     */
    // using thread_callback = void*(void*);

    // private:
    // 这个线程函数设置在这里有问题，我想用exec函数族，但是这个函数是替换进程，不能用在线程身上，但是保留，做一个线程的参考
    /**
     * @brief 定义_deal_tree线程处理的回调函数
     * @brief 这个函数需要是静态的，因为在类的内部传递函数指针不能依赖于类的实例化对象，所以需要定义成静态函数来使用作用域::就能访问到
     * @param args，接受的参数
     * @return void*，返回子线程执行的状态
     */
    // static void* _tree_callback(void* args);
    // static thread_callback _tree_callback;

    //------------------------------------------------------------

private:
    //***************************下面就是具体业务的函数逻辑***************************

    /**
     * @brief 处理Show类型命令
     */
    void _deal_show();

    /**
     * @brief 处理Tree类型命令
     */
    void _deal_tree();

    /**
     * @brief 处理Quit类型命令
     */
    void _deal_quit();

    /**
     * @brief 处理Clear类型命令
     */
    void _deal_clear();

    /**
     * @brief 处理Create_Database类型命令
     */
    void _deal_create_database();

    /**
     * @brief 处理Drop_Database类型命令
     */
    void _deal_drop_database();

    /**
     * @brief 处理Use类型命令
     */
    void _deal_use();

    /**
     * @brief  在处理表的命令的时候都需要判断是否选中数据库
     * @return true
     * @return false
     */
    bool _check_if_use();

    /**
     * @brief 处理Create_Table类型命令
     */
    void _deal_create_table();

    /**
     * @brief 处理Drop_Table类型命令
     */
    void _deal_drop_table();

    /**
     * @brief 处理Select类型命令
     */
    void _deal_select();

    /**
     * @brief 处理Delete类型命令
     */
    void _deal_delete();

    /**
     * @brief 处理Insert类型命令
     */
    void _deal_insert();

    /**
     * @brief 处理Update类型命令
     */
    void _deal_update();

    /**
     * @brief 处理Unknown类型命令
     */
    void _deal_unknown();

public:
    /**
     * @brief 文件目录或者文件名当中不能出现的字符集合
     */
    static const std::vector<char> banned_ch;

    /**
     * @brief 存放数据库的相对目录前缀
     */
    static const std::string data_prefix;

    /**
     * @brief 存放资源文件的相对目录前缀
     */
    static const std::string resources_prefix;

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
     * @brief 存储当前使用的数据库名称
     */
    std::string m_dbname;

    /**
     * @brief 存储处理完客户端命令之后的反馈字符串
     */
    std::string m_feedback;
};

#endif
