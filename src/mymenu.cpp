/**
 * @file menu.cpp
 * @brief 菜单类的源文件
 * @author lzx0626 (2065666169@qq.com)
 * @version 1.0
 * @date 2023-10-19
 *
 * @copyright Copyright (c) 2023  电子科技大学
 *
 */

#include "mymenu.h"

/**
 * @brief 对类内函数的实现
 */

void Menu::run() {
    // 在../resources目录中存放着menu.txt，里面的内容就是数据库系统进入之后的提示内容
    _open_print("../resources/menu_start.txt");

    // 读入命令
    while (1)
        if (_command_input())
            break;

    // 结束，打印结束消息
    _open_print("../resources/menu_end.txt");
}

void Menu::_open_print(const std::string& path) {
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

bool Menu::_command_input() {
    puts("请输入命令: ");  // puts()自带换行符

    // 一个一个字符读入，如果遇到';'则结束，将得到的命令存储在command中
    command.clear();

    while (1) {
        // 我们在输入的过程中对输入的字符串进行格式化，最重要的一点就是去掉没有必要的空格
        int ch = fgetc(stdin);
        // 我们个人规定不允许使用\t缩进符号，所以我们把它替换为空格
        if ('\t' == ch)
            ch = ' ';

        // 1.当字符串为空的时候输入空格或者回车将被忽略
        if (command.empty() && (' ' == ch || '\n' == ch))
            continue;
        // 2.同第一点，当换行的时候也不允许出现类似情况
        if ('\n' == command.back() && (' ' == ch || '\n' == ch))
            continue;
        // 3.当上一个字符是空格的时候再次输入空格就被忽略
        if (' ' == command.back() && ch == ' ')
            continue;

        // 遇到分号结束输入
        if (';' == ch)
            break;

        command += ch;

        // 在这里做退出的判断，退出的命令格式只能是"q\n"或者"quit\n"，包含空格和\t的话还有一些
        if ("q\n" == command || "quit\n" == command ||
            "q \n" == command || "quit \n" == command)
            return true;
    }
    // std::cout << command << std::endl;

    cmd_type = _get_type(command);
    // std::cout << cmd_type << std::endl;

    puts("");

    return false;
}

Menu::Command_Type Menu::_get_type(const std::string& command) {
    // 经过我们输入的处理之后字符串的开头肯定是有含义的字符，所以实现这个函数用于得到命令的类型

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

Menu::Command_Type Menu::_get_database_table(size_t pos, const std::string& command, bool first) {
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
