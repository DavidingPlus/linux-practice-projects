/**
 * @file Menu.cpp
 * @brief 聊天室启动的终端界面的菜单，源文件
 * @author lzx0626 (2065666169@qq.com)
 * @version 1.0
 * @date 2023-09-18
 *
 * @copyright Copyright (c) 2023  电子科技大学
 *
 */

#include "main_menu.h"

/**
 * @brief 对类内成员函数的实现
 */
Menu::Menu(int connect_fd) {
    _init(connect_fd);
}

void Menu::run(call_back call) {
    // 先打印一段话
    std::cout << "                  简易的聊天室                  " << std::endl;
    std::cout << "-------------------------------------------------" << std::endl;

    while (1) {
        std::cout << "请输入: " << std::endl;
        std::cout << "(1)匿名聊天     (2)登录     (3)注册     (4)退出" << std::endl;

        // 等待用户输入
        int choice = -1;
        std::cin >> choice;
        getchar();  // 把cin输入的回车吃掉

        if (1 == choice) {
            _anonymous_chat(call);
            break;
        } else if (2 == choice) {
            _sign_in(call);
            std::cout << std::endl;  // 只有登录失败才会到达这个地方
        } else if (3 == choice)
            _sign_up();
        else if (4 == choice)
            __exit();
        else
            _error();
    }
}

void Menu::_init(int connect_fd) {
    // 工作目录是build目录，从这个目录开始算
    file_path = std::string("../src/username_password.txt");
    // 初始化文件描述符
    this->connect_fd = connect_fd;
}

void Menu::_anonymous_chat(call_back call) {
    // 随机生成一个0到99999的随机数
    srand((unsigned)time(nullptr));

    int num = rand() % 100000;
    char name[Max_Buffer_Size] = {0};
    sprintf(name, "匿名用户%d", num);

    this->username = name;

    call(username, connect_fd);
}

void Menu::_sign_in(call_back call) {
    // 让用户输入用户名和密码
    char _username[Max_Buffer_Size] = {0};
    char _password[Max_Buffer_Size] = {0};

    std::cout << "请输入用户名和密码(中间用换行隔开): " << std::endl;

    fgets(_username, sizeof(_username), stdin);
    fgets(_password, sizeof(_password), stdin);

    // 输入的用户名密码，带有换行符，给他去掉
    std::string Username = std::string(_username);
    Username.pop_back();
    std::string Password = std::string(_password);
    Password.pop_back();

    if (!_if_exists(Username, Password)) {
        std::cout << "用户名或者密码错误!!!" << std::endl;
        return;
    }

    // 成功
    std::cout << "恭喜，登录成功!" << std::endl;

    call(username, connect_fd);
}

void Menu::_sign_up() {
    // 打开文件
    FILE* file_stream = fopen(file_path.c_str(), "a");
    if (!file_stream) {
        perror("fopen");
        exit(-1);
    }

    // 输入用户名
    std::cout << "请输入您的用户名: " << std::endl;

    char Username[Max_Buffer_Size] = {0};
    fgets(Username, sizeof(Username), stdin);  // 注意输入的带有'\n'
    fwrite(Username, 1, strlen(Username) - 1, file_stream);

    // 写一个空格
    fputc(' ', file_stream);
    // 这里写了之后需要重置一下文件指针，否则下面写不出来
    fseek(file_stream, SEEK_END, 0);

    // 输入密码
    std::cout << "请输入您的密码: " << std::endl;

    char Password[Max_Buffer_Size] = {0};
    fgets(Password, sizeof(Password), stdin);  // 注意输入的带有'\n'
    fwrite(Password, 1, strlen(Password) - 1, file_stream);

    // 最后我们规定写一个换行符号，用于区分两个账号
    fputc('\n', file_stream);

    // 关闭文件流
    fclose(file_stream);

    // 输出一段消息
    std::cout << "注册成功，请重新登录!" << std::endl
              << std::endl;
}

void Menu::__exit() {
    std::cout << "欢迎下次使用!!!" << std::endl;
    exit(0);
}

void Menu::_error() {
    std::cout << "输入错误请重新输入!" << std::endl;
}

bool Menu::_if_exists(const std::string& Username, const std::string& Password) {
    // 打开文件进行搜索
    FILE* file_stream = fopen(file_path.c_str(), "r");
    if (!file_stream) {
        perror("fopen");
        exit(-1);
    }

    // 用户名和密码的存储形式是一行一个用户，然后用户名在前，密码在后，中间用空格隔开
    // 当然我们在创建用户的时候也就不允许使用空格作为用户名或者密码
    char content[Max_Buffer_Size] = {0};
    while (1) {
        // 读取数据
        bzero(content, sizeof(content));
        fgets(content, sizeof(content), file_stream);

        // 如果为空则退出
        if ('\0' == content[0])
            break;

        // 通过文件按照行缓冲的方式读取的字符串含有换行符，也就是账号与账号之间的间隔
        std::string infile = std::string(content);
        infile.pop_back();
        int pos = infile.find(' ');

        // 开始搜索
        // 例如 admin 123456 ('\n'已经被弹掉)
        // 用户名和密码均匹配才成功
        if (Username == std::string(infile.begin(), infile.begin() + pos) &&
            Password == std::string(infile.begin() + pos + 1, infile.end())) {
            username = Username;
            password = Password;

            fclose(file_stream);
            return true;
        }
    }

    // 关闭文件流
    fclose(file_stream);

    return false;
}
