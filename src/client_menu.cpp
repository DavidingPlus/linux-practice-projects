/**
 * @file client_menu.cpp
 * @brief 客户端菜单类的源文件
 * @author lzx0626 (2065666169@qq.com)
 * @version 1.0
 * @date 2023-10-19
 *
 * @copyright Copyright (c) 2023  电子科技大学
 *
 */

#include "client_menu.h"

/**
 * @brief 对类内函数的实现
 */

std::string Menu::run() {
    // 只有第一次会主动打印这些
    if (0 == count++)
        // 在../resources目录中存放着menu.txt，里面的内容就是数据库系统进入之后的提示内容
        Tools::open_and_print(Order::resources_prefix + "menu_start.txt");

    // 以下是输入命令并且处理命令字符串的逻辑
    puts("请输入命令: ");  // puts()自带换行符

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

    return command;
}
