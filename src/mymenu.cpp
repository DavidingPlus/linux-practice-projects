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

bool Menu::_command_input() {
    puts("请输入命令: ");  // puts()自带换行符

    // 一个一个字符读入，如果遇到';'则结束，将得到的命令存储在command中，等命令输入结束之后在Order中进行设置
    order.clear();

    std::string command;

    while (1) {
        // 我们在输入的过程中对输入的字符串进行格式化，最重要的一点就是去掉没有必要的空格
        int ch = fgetc(stdin);

        // 我个人不允许使用缩进'\t'和回车'\n'将其替换为' '，后面的空格可以代表这三个
        if ('\t' == ch or '\n' == ch)
            ch = ' ';

        // 1.当字符串为空的时候输入空格将被忽略
        if (command.empty() and ' ' == ch)
            continue;
        // 2.当上一个字符是空格的时候再次输入空格就被忽略
        if (' ' == command.back() and ' ' == ch)
            continue;

        // 遇到分号结束输入
        if (';' == ch)
            break;

        command += ch;
    }

    // command最后很可能出现一个空格，因为本来等待下一个字符，然后就结束了，如果有需要将其弹掉
    if (' ' == command.back())
        command.pop_back();

    // 命令输入结束之后将Order中的命令字符串修改
    order.set_command(command);

    // 执行这条命令
    // 注意，我们的退出是在菜单里面做的，因为我自己认为没必要放到Order类当中
    order.run();

    puts("");

    return false;  // 这里的true和false是针对前面退出程序的判断来的，没有实际的逻辑含义
}
