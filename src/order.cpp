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

std::string Order::database_prefix = "../data/";

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

void Order::set_command(const std::string& order) {
    m_command = order;
}

void Order::run() {
    std::cout << std::endl;

    // 首先肯定是要判断命令的类型
    // _get_type确定的是一个初步的类型，就是这个命令可能是属于这一个，具体是否属于我们调用针对性的处理函数就可以了
    m_command_type = _get_type(m_command);

    switch (m_command_type) {
    case Show:
        _deal_show();
        break;
    case Tree:
        _deal_tree();
        break;
    case Quit:
        _deal_quit();
        break;
    case Clear:
        _deal_clear();
        break;
    case Create_Database:
        _deal_create_database();
        break;
    case Drop_Database:
        _deal_drop_database();
        break;
    case Use:
        _deal_use();
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
    // 退出命令，展示命令，Tree命令查看所有，Clear命令 没有空格，我们直接在这里判断即可
    if ("q" == command or "quit" == command)
        return Command_Type::Quit;
    if ("show" == command)
        return Command_Type::Show;
    if ("tree" == command)
        return Command_Type::Tree;
    if ("clear" == command)
        return Command_Type::Clear;

    // 在众多命令当中，只有create和drop是可以分为两种情况的，作用于数据库和表
    size_t pos = command.find(' ');
    std::string sub_cmd = command.substr(0, pos);

    // std::cout << sub_cmd << std::endl;

    if ("tree" == sub_cmd)  // tree <dbname>
        return Command_Type::Tree;
    else if ("create" == sub_cmd)
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

// show
void Order::_deal_show() {
    // 同退出的逻辑一样，进入这里一定是正确的命令
    _open_print("../resources/menu_start.txt");
}

// tree / tree <dbname>
void Order::_deal_tree() {
    // 首先判断命令是否为正确的tree命令
    // tree 或者 tree <dbname>，因此出现两个或者两个以上的括号是不合法的
    std::string dbname = std::string();

    size_t pos = m_command.find(' ');
    if (std::string::npos != pos) {
        // 查询第二个空格
        std::string sub_cmd = m_command.substr(pos + 1, m_command.size());
        if (std::string::npos != sub_cmd.find(' ')) {
            _deal_unknown();
            return;
        }
        // 有空格出现，说明想查询指定的数据库架构
        dbname = sub_cmd;
    }
    // 创建一个子进程
    pid_t pid = fork();
    if (-1 == pid) {
        perror("fork");
        exit(-1);
    }

    if (pid > 0) {
        // 父进程，阻塞等待回收子进程
        int ret = waitpid(-1, nullptr, 0);
        if (-1 == ret) {
            perror("waitpid");
            exit(-1);
        }
    } else if (0 == pid) {
        // 判断这个数据库存不存在
        std::string path = database_prefix + dbname;
        if (0 != access(path.c_str(), F_OK)) {
            std::cout << "您指定的数据库不存在,请检查之后重新输入!" << std::endl;
            // 结束子进程，否则子进程去跑父进程的菜单代码了，会紊乱
            exit(0);
        }

        std::cout << "数据库目录架构如下所示: " << std::endl;
        // 子进程逻辑，调用exec函数族执行tree命令
        execlp("tree", "tree", path.c_str(), "-I", "README.md", nullptr);  // 忽略目录中引导作用的README.md
    }
}

// q / quit
void Order::_deal_quit() {
    // 从上面的逻辑判断，这个东西一定是对的指令
    _open_print("../resources/menu_end.txt");
    exit(0);
}

// clear
void Order::_deal_clear() {
    // 调用exec函数族清空屏幕
    pid_t pid = fork();
    if (-1 == pid) {
        perror("fork");
        exit(-1);
    }

    if (pid > 0) {
        // 父进程，阻塞等待回收子进程
        int ret = waitpid(-1, nullptr, 0);
        if (-1 == ret) {
            perror("waitpid");
            exit(-1);
        }
    } else if (0 == pid)
        // 清空屏幕
        execlp("clear", "clear", nullptr);
}

// create database <dbname>
void Order::_deal_create_database() {
    // 进行更细致的判断
    // 前面的几个字符一定是 "create database"
    size_t pos = strlen("create database");
    // 由于我们把末尾的空格(如果存在)给弹掉了，所以现在正确的字符串第一个字符是空格，然后后面不存在空格了
    if (' ' != m_command[pos]) {
        _deal_unknown();
        return;
    }

    // 现在该拿到目录的name了
    std::string name = m_command.substr(pos + 1, m_command.size());
    if (std::string::npos != name.find(' ')) {
        _deal_unknown();
        return;
    }

    // 我想要把数据库创建在data目录中，需要做特殊字符的判断
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
        std::cout << "该命名的数据库已存在,请检查名称并修改!" << std::endl;
    else {
        mkdir(path.c_str(), 0755);
        std::cout << "数据库创建成功! 名称: " << name << std::endl;
    }
}

// drop database <dbname>
void Order::_deal_drop_database() {
    // 大体的逻辑同创建数据库一样
    size_t pos = strlen("drop database");
    if (' ' != m_command[pos]) {
        _deal_unknown();
        return;
    }

    std::string name = m_command.substr(pos + 1, m_command.size());
    if (std::string::npos != name.find(' ')) {
        _deal_unknown();
        return;
    }

    // 得到数据库名字，先看存不存在
    std::string path = database_prefix + name;
    if (0 != access(path.c_str(), F_OK)) {
        std::cout << "目标数据库不存在,请检查名称并修改!" << std::endl;
        return;
    }
    // 检查目录是否为空
    DIR* dir = opendir(path.c_str());
    if (nullptr == dir) {
        perror("opendir");
        exit(-1);
    }

    // 循环读取目录中的文件项内容，如果除了"."和".."以外的就是非空，注意名字里面没有 /
    while (1) {
        // 读到末尾或者错误都返回nullptr，为了区分，man文档建议我们把errno设置为0，没变就是没错误
        errno = 0;
        struct dirent* file = readdir(dir);
        if (nullptr == file) {
            if (0 != errno) {
                perror("readdir");
                exit(-1);
            }
            // 退出
            break;
        }

        // 判断是否为空
        // std::cout << file->d_name << std::endl;
        if ("." != std::string(file->d_name) and ".." != std::string(file->d_name)) {
            std::cout << "目标数据库不为空,请将数据库清空之后再次尝试!" << std::endl;
            return;
        }
    }
    // 删除目录
    rmdir(path.c_str());  // rmdir只能删除空目录，虽然可以通过错误号判断是错误还是非空目录，但是还是从上面的代码来吧
    std::cout << "目标数据库删除成功!" << std::endl;
}

// use <dbname>
void Order::_deal_use() {
    // 已经有一个空格了，剩下的部分不能存在空格
    size_t pos = strlen("use");
    std::string sub_cmd = m_command.substr(pos + 1, m_command.size());
    if (std::string::npos != sub_cmd.find(' ')) {
        _deal_unknown();
        return;
    }
    // 判断这个数据库存不存在
    std::string path = database_prefix + sub_cmd;
    if (0 != access(path.c_str(), F_OK)) {
        std::cout << "您指定的数据库不存在,请检查之后重新输入!" << std::endl;
        return;
    }

    // 更改使用的数据库目录
    m_dbname = sub_cmd;
    std::cout << m_dbname << std::endl;
}

void Order::_deal_unknown() {
    m_command_type = Command_Type::Unknown;
    m_dbname = std::string();

    std::cout << "您输入的命令不存在或者不正确,请检查之后重新输入!" << std::endl;
}
