/**
 * @file order.cpp
 * @brief 处理输入命令类的源文件
 * @author lzx0626 (2065666169@qq.com)
 * @version 1.0
 * @date 2023-10-20
 *
 * @copyright Copyright (c) 2023  电子科技大学
 *
 */

#include "order.h"

/**
 * @brief 初始化类内静态变量
 */
// '\\'其中一个\做转义字符，真正的字符是'\'
std::vector<char> Order::banned_ch = {'\\', '/', ':', '*', '?', '"', '<', '>', '|'};

std::string Order::database_prefix = "../DB/";

/**
 * @brief 实现extern类别函数_open_print
 */
void _open_print(const std::string& path) {
    // 打开文件
    FILE* file = fopen(path.c_str(), "r");
    if (nullptr == file) {
        perror("fopen");
        exit(-1);
    }

    // 读取内容
    char read_buf[BUFSIZ] = {0};  // 数组容量用系统默认给的缓冲区大小 8192
    while (1) {
        bzero(read_buf, BUFSIZ);
        size_t len = fread(read_buf, 1, BUFSIZ - 1, file);
        // 返回0表明可能出错或者读到末尾了
        if (0 == len) {
            if (ferror(file)) {
                perror("fread");
                exit(-1);
            }
            if (feof(file))
                break;
        }

        // 打印到屏幕上
        fwrite(read_buf, 1, len, stdout);
    }

    // 关闭
    fclose(file);
}

/**
 * @brief 对类内函数的实现
 */

void Order::clear() {
    m_command.clear();
    m_command_type = Unknown;
}

std::string Order::get_command() {
    return m_command;
}

void Order::set_command(const std::string& order) {
    m_command = order;
}

void Order::run() {
    // 首先肯定是要判断命令的类型
    // _get_type确定的是一个初步的类型，就是这个命令可能是属于这一个，具体是否属于我们调用针对性的处理函数就可以了
    m_command_type = _get_type(m_command);

    switch (m_command_type) {
    case Show:
        _deal_show();
        break;
    case Quit:
        _deal_quit();
        break;
    case Create_Database:
        _deal_create_database();
        break;
    case Drop_Database:
        break;
    case Use:
        break;
    case Create_Table:
        break;
    case Drop_Table:
        break;
    case Select:
        break;
    case Delete:
        break;
    case Insert:
        break;
    case Update:
        break;
    case Unknown:
        _deal_unknown();
        break;
    }
}

Order::Command_Type Order::_get_type(const std::string& command) {
    // 经过我们输入的处理之后字符串的开头肯定是有含义的字符，所以实现这个函数用于得到命令的类型
    // 退出命令和展示命令找不到空格，我们直接在这里判断即可
    if ("q" == command or "quit" == command)
        return Command_Type::Quit;
    if ("show" == command)
        return Command_Type::Show;

    // 在众多命令当中，只有create和drop是可以分为两种情况的，作用于数据库和表
    size_t pos = command.find(' ');
    std::string sub_cmd = command.substr(0, pos);

    // std::cout << sub_cmd << std::endl;

    if ("create" == sub_cmd)
        return _get_database_table(pos, command, true);
    else if ("drop" == sub_cmd)
        return _get_database_table(pos, command, false);
    else if ("use" == sub_cmd)
        return Command_Type::Use;
    else if ("select" == sub_cmd)
        return Command_Type::Select;
    else if ("delete" == sub_cmd)
        return Command_Type::Delete;
    else if ("insert" == sub_cmd)
        return Command_Type::Insert;
    else if ("update" == sub_cmd)
        return Command_Type::Update;
    else
        return Command_Type::Unknown;
}

Order::Command_Type Order::_get_database_table(size_t pos, const std::string& command, bool first) {
    // 找到第二空格确定到底是哪一个
    std::string right_command = command.substr(pos + 1, command.size());
    size_t pos2 = right_command.find(' ');
    std::string sub_cmd2 = right_command.substr(0, pos2);
    // 进行判断
    if ("database" == sub_cmd2)
        return first ? Command_Type::Create_Database : Command_Type::Drop_Database;
    else if ("table" == sub_cmd2)
        return first ? Command_Type::Create_Table : Command_Type::Drop_Table;
    else
        return Command_Type::Unknown;
}

void Order::_deal_quit() {
    // 从上面的逻辑判断，这个东西一定是对的指令
    _open_print("../resources/menu_end.txt");
    exit(0);
}

void Order::_deal_show() {
    // 同退出的逻辑一样，进入这里一定是正确的命令
    _open_print("../resources/menu_start.txt");
}

void Order::_deal_create_database() {
    // 进行更细致的判断
    // 前面的几个字符一定是 "create database"
    size_t pos = strlen("create database");

    // 由于我们把末尾的空格(如果存在)给弹掉了，所以现在正确的字符串第一个字符是空格，然后后面不存在空格了
    if (' ' != m_command[pos]) {
        _deal_unknown();
        return;
    }

    // 现在该拿到目录的path了
    std::string name = m_command.substr(pos + 1, m_command.size());
    if (std::string::npos != name.find(' ')) {
        _deal_unknown();
        return;
    }

    // 我想要把数据库创建在DB目录中，需要做特殊字符的判断
    // 不能出现 \ / : * ? " < > |
    for (auto& ch : banned_ch)
        if (std::string::npos != name.find(ch)) {
            std::cout << "数据库名字当中带有非法字符,请重新输入!" << std::endl;
            return;
        }

    // 然后开始创建数据库，就是创建一个目录
    // 先判断目录是否存在
    std::string path = database_prefix + name;
    if (0 == access(path.c_str(), F_OK))
        std::cout << "数据库已存在,请检查名称并修改!" << std::endl;
    else {
        mkdir(path.c_str(), 0755);
        std::cout << "数据库创建成功!" << std::endl;
    }
}

void Order::_deal_unknown() {
    m_command_type = Command_Type::Unknown;

    std::cout << "您输入的命令不存在或者不正确,请检查之后重新输入!" << std::endl;
}
