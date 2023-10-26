/**
 * @file server_order.cpp
 * @brief 服务端处理输入命令的类的头文件
 * @author lzx0626 (2065666169@qq.com)
 * @version 1.0
 * @date 2023-10-20
 *
 * @copyright Copyright (c) 2023  电子科技大学
 *
 */

#include "server_order.h"

/**
 * @brief 初始化类内静态变量
 */
// '\\'其中一个\做转义字符，真正的字符是'\'
const std::vector<char> Order::banned_ch = {'\\', '/', ':', '*', '?', '"', '<', '>', '|'};

const std::string Order::data_prefix = "../data/";

const std::string Order::resources_prefix = "../resources/";

/**
 * @brief 对类内函数的实现
 */
void Order::set_command(const std::string& order) {
    // 先清空类内部的对象(m_dbname不要清空，我们要保存并且记录),因为这是一条命令处理的开始
    m_command.clear();
    m_command_type = Order::Unknown;
    m_feedback.clear();

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
        _deal_create_table();
        break;
    case Drop_Table:
        _deal_drop_table();
        break;
    case Select:
        _deal_select();
        break;
    case Delete:
        _deal_delete();
        break;
    case Insert:
        _deal_insert();
        break;
    case Update:
        _deal_update();
        break;
    case Unknown:
        _deal_unknown();
        break;
    }
}

void Order::read_feedback() {
    // 打开文件
    FILE* file = fopen(std::string(Order::resources_prefix + "feedback.txt").c_str(), "r");
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

        // 添加到m_feedback中
        m_feedback += read_buf;
    }

    // 关闭
    fclose(file);
}

Order::Command_Type Order::_get_type(const std::string& command) {
    // 经过我们输入的处理之后字符串的开头肯定是有含义的字符，所以实现这个函数用于得到命令的类型
    // 退出命令，show命令，tree命令查看所有，clear命令没有空格，我们直接在这里判断即可
    if ("show" == command)
        return Command_Type::Show;
    if ("q" == command or "quit" == command)
        return Command_Type::Quit;
    if ("tree" == command)
        return Command_Type::Tree;
    if ("clear" == command)
        return Command_Type::Clear;

    // 在众多命令当中，只有create和drop是可以分为两种情况的，作用于数据库和表
    // 这里我要说明一下，对于错误的指令，比如create;找不到空格，这里pos就是npos，这里就会进入unknown的处理，很合理
    size_t pos = command.find(' ');
    if (std::string::npos == pos)  // 一个空格都没有那肯定是不对的命令，特例都在上面判断了
        return Command_Type::Unknown;

    // std::string command_for_type = command.substr(0, pos);
    // substr第二个参数不是末尾的位置，而是从上一个位置开始的长度!!!所以我用构造函数了
    std::string command_for_type = std::string(m_command.begin(), m_command.begin() + pos);
    // std::cout << command_for_type << std::endl;

    if ("tree" == command_for_type)  // tree <dbname>
        return Command_Type::Tree;
    else if ("create" == command_for_type)
        return _get_database_table(pos, command, true);
    else if ("drop" == command_for_type)
        return _get_database_table(pos, command, false);
    else if ("use" == command_for_type)
        return Command_Type::Use;
    else if ("select" == command_for_type)
        return Command_Type::Select;
    else if ("delete" == command_for_type)
        return Command_Type::Delete;
    else if ("insert" == command_for_type)
        return Command_Type::Insert;
    else if ("update" == command_for_type)
        return Command_Type::Update;
    else
        return Command_Type::Unknown;
}

Order::Command_Type Order::_get_database_table(size_t blank_pos, const std::string& command, bool first) {
    // 找到第二空格确定到底是哪一个
    std::string right_command = std::string(m_command.begin() + blank_pos + 1, m_command.end());
    size_t pos = right_command.find(' ');
    if (std::string::npos == pos)
        return Command_Type::Unknown;

    std::string command_for_second_type = std::string(right_command.begin(), right_command.begin() + pos);
    // std::cout << command_for_second_type << std::endl;

    // 进行判断
    if ("database" == command_for_second_type)
        return first ? Command_Type::Create_Database : Command_Type::Drop_Database;
    else if ("table" == command_for_second_type)
        return first ? Command_Type::Create_Table : Command_Type::Drop_Table;
    else
        return Command_Type::Unknown;
}

// show
void Order::_deal_show() {
    // 同退出的逻辑一样，进入这里一定是正确的命令
    open_and_print(resources_prefix + "menu_start.txt");
}

// tree / tree <dbname>
void Order::_deal_tree() {
    // 首先判断命令是否为正确的tree命令
    // tree 或者 tree <dbname>，因此出现两个或者两个以上的括号是不合法的
    std::string dbname = std::string();

    size_t pos = m_command.find(' ');
    if (std::string::npos != pos) {
        // 查询第二个空格
        std::string command_dbname = std::string(m_command.begin() + pos + 1, m_command.end());
        if (std::string::npos != command_dbname.find(' ')) {
            _deal_unknown();
            return;
        }
        // 有空格出现，说明想查询指定的数据库架构
        dbname = command_dbname;
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
        std::string path = data_prefix + dbname;
        if (0 != access(path.c_str(), F_OK)) {
            std::cout << "数据库 " << dbname << " 不存在,请检查之后重新输入!" << std::endl;
            // 结束子进程，否则子进程去跑父进程的菜单代码了，会紊乱
            exit(0);
        }

        std::cout << "数据库目录架构如下所示: " << std::endl;
        // 子进程逻辑，调用exec函数族执行tree命令
        execlp("tree", "tree", path.c_str(), "-a", "-I", "README.md", nullptr);  // 忽略目录中引导作用的README.md
    }
}

// q / quit
void Order::_deal_quit() {
    // 从上面的逻辑判断，这个东西一定是对的指令
    open_and_print(resources_prefix + "menu_end.txt");
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
        // TODO
        // 这里我没有更改逻辑，但是服务端调用的东西能让客户端清屏，我需要研究一下clear命令的实质了...
        execlp("clear", "clear", nullptr);
}

// create database <dbname>
void Order::_deal_create_database() {
    // 前面的几个字符一定是 "create database"，并且一定存在第三个参数!
    size_t pos = strlen("create database");
    // 现在该拿到目录的name了
    std::string command_dbname = std::string(m_command.begin() + pos + 1, m_command.end());
    if (std::string::npos != command_dbname.find(' ')) {
        _deal_unknown();
        return;
    }

    // 我想要把数据库创建在data目录中，需要做特殊字符的判断
    // 不能出现 \ / : * ? " < > |
    if (check_has_any(command_dbname, banned_ch)) {
        std::cout << "数据库命名 \"" << command_dbname << "\" 当中带有非法字符,请重新输入!" << std::endl;
        return;
    }

    // 然后开始创建数据库，就是创建一个目录
    std::string path = data_prefix + command_dbname;
    if (0 == access(path.c_str(), F_OK))  // 先判断目录是否存在
        std::cout << "数据库 " << command_dbname << " 已存在,请检查名称并修改!" << std::endl;
    else {
        mkdir(path.c_str(), 0755);
        std::cout << "数据库 " << command_dbname << " 创建成功!" << std::endl;
    }
}

// drop database <dbname>
void Order::_deal_drop_database() {
    // 大体的逻辑同创建数据库一样
    size_t pos = strlen("drop database");
    std::string command_dbname = std::string(m_command.begin() + pos + 1, m_command.end());
    if (std::string::npos != command_dbname.find(' ')) {
        _deal_unknown();
        return;
    }

    // 得到数据库名字，先看存不存在
    std::string path = data_prefix + command_dbname;
    if (0 != access(path.c_str(), F_OK)) {
        std::cout << "数据库 " << command_dbname << " 不存在,请检查名称并修改!" << std::endl;
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
            std::cout << "数据库 " << command_dbname << " 不为空,请将数据库清空之后再次尝试!" << std::endl;
            return;
        }
    }
    // 删除目录
    rmdir(path.c_str());  // rmdir只能删除空目录，虽然可以通过错误号判断是错误还是非空目录，但是还是从上面的代码来吧
    std::cout << "数据库 " << command_dbname << " 删除成功!" << std::endl;
}

// use <dbname>
void Order::_deal_use() {
    // 已经有一个空格了，剩下的部分不能存在空格
    size_t pos = strlen("use");
    std::string command_dbname = std::string(m_command.begin() + pos + 1, m_command.end());
    if (std::string::npos != command_dbname.find(' ')) {
        _deal_unknown();
        return;
    }
    // 判断这个数据库存不存在
    std::string path = data_prefix + command_dbname;
    if (0 != access(path.c_str(), F_OK)) {
        m_dbname.clear();  // 清空数据库名字数据
        std::cout << "数据库 " << command_dbname << " 不存在,请检查之后重新输入!" << std::endl;
        return;
    }

    // 更改使用的数据库目录
    m_dbname = command_dbname;
    std::cout << "已切换到数据库 " << m_dbname << std::endl;
}

/*******关于表的操作都必须在选中数据库之前，所以需要先进行判断*******/

bool Order::_check_if_use() {
    if (m_dbname.empty()) {
        std::cout << "未选择任何数据库!请选择合适数据库之后重试!" << std::endl;
        return false;
    }
    return true;
}

// create table <table_name> ( <column> <type> ,...);
// 写好的屎山，就不要动它了...
void Order::_deal_create_table() {
    // 进来就检测是否选中数据库
    if (!_check_if_use())
        return;

    // 实例化Table对象
    Table table;

    size_t pos = strlen("create table");
    // "create table"的错误命令在上面判断过了，这里不判断
    // 查询 '(' 和 ')'
    size_t pos_left = m_command.find('(');
    if (std::string::npos == pos_left) {
        _deal_unknown();
        return;
    }
    size_t pos_right = m_command.find(')');
    if (std::string::npos == pos_right) {
        _deal_unknown();
        return;
    }

    // 如果 ')' 后面出现其他字符则命令不正确
    if (')' != m_command.back()) {
        _deal_unknown();
        return;
    }

    // 处理table_name
    if (pos + 1 == pos_left) {  // 到这一步table后面一定带有一个空格了，否则就是unknown命令
        _deal_unknown();
        return;
    }

    std::string table_name = std::string(m_command.begin() + pos + 1, m_command.begin() + pos_left);
    // 如果<table_name>和 '(' 中间有一个空格，我们把它弹掉
    if (' ' == m_command[pos_left - 1])
        table_name.pop_back();
    // 如果末尾的空格都被弹掉之后，table_name中还有空格则命令不对
    if (std::string::npos != table_name.find(' ')) {
        _deal_unknown();
        return;
    }

    // 检测表名是否符合命名规范
    // 不能出现 \ / : * ? " < > |
    if (check_has_any(table_name, banned_ch)) {
        std::cout << "表命名 \"" << table_name << "\" 当中带有非法字符,请重新输入!" << std::endl;
        return;
    }

    table.m_table_name = table_name;
    // std::cout << '(' << table_name << ')' << std::endl;

    // 判断表是否已经存在
    std::string path = Order::data_prefix + m_dbname + '/' + table_name + ".dat";
    if (0 == access(path.c_str(), F_OK)) {
        std::cout << "表 " << table.m_table_name << " 已存在,请检查名称并修改!" << std::endl;
        return;
    }

    // 处理column_name和column_type
    std::string column_string = std::string(m_command.begin() + pos_left + 1, m_command.begin() + pos_right);
    // 在处理之前，我们把收尾的空格弹掉(如果存在)，方便判断最后一个列是否具有 ','
    pop_space(column_string);

    // 判断 ','
    if (',' == column_string.back()) {
        std::cout << "最后一列末尾不需要 ',' !请检查之后重试!" << std::endl;
        return;
    }

    // 现在按 ',' 进行分割
    auto type_name_s = my_spilt(column_string, ',');
    for (auto& row : type_name_s) {
        // 处理每一行
        // 先把首尾的空格弹掉
        pop_space(row);
        // std::cout << '(' << row << ')' << std::endl;

        // 现在的数据只可能是 "<column> <type>"，也就是只有一个空格
        std::vector<std::string> type_name = my_spilt(row, ' ');
        if (2 != type_name.size()) {
            _deal_unknown();
            return;
        }

        // 检查名称
        if (check_has_any(type_name[0], banned_ch)) {
            std::cout << "字段名称 \"" << type_name[0] << "\" 当中含有非法字符,请重新输入!" << std::endl;
            return;
        }
        // 检查类型，目前只考虑是int或者string
        if ("int" != type_name[1] and "string" != type_name[1]) {
            std::cout << "字段类型 \"" << type_name[1] << "\" 不符合规范,请重新输入" << std::endl;
            return;
        }
        // 存储
        table.m_columns.push_back({type_name[0], type_name[1]});
    }

    // 存储到文件中，path在前面已经定义
    // Table结构体里面使用了vector，导致大小不确定，如果直接写入结构体，在读取的时候新的Table不知道大小是多少，会段错误
    // 因此在写入的时候我需要执行相关的规则才能保证正确的写入
    write_table_to_file(table, path);

    // 输出反馈
    std::cout << "表 " << table.m_table_name << " 创建成功!" << std::endl;
}

// drop table <table_name>
void Order::_deal_drop_table() {
    if (!_check_if_use())
        return;

    // 大体的逻辑同删除数据库一样
    size_t pos = strlen("drop table");
    std::string command_table_name = std::string(m_command.begin() + pos + 1, m_command.end());
    if (std::string::npos != command_table_name.find(' ')) {
        _deal_unknown();
        return;
    }

    // 得到表名字，先看存不存在
    std::string path = data_prefix + m_dbname + "/" + command_table_name + ".dat";
    if (0 != access(path.c_str(), F_OK)) {
        std::cout << "表 " << command_table_name << " 不存在,请检查名称并修改!" << std::endl;
        return;
    }

    // 删除文件
    int ret = unlink(path.c_str());
    if (-1 == ret) {
        perror("unlink");
        exit(-1);
    }

    std::cout << "表 " << command_table_name << " 删除成功!" << std::endl;
}

// select <column> from <table> [where <cond>]
// 写好的屎山，就不要动它了...
void Order::_deal_select() {
    if (!_check_if_use())
        return;

    // 实例化Table对象
    Table table;

    size_t pos = strlen("select");
    size_t pos_from = m_command.find("from");
    if (std::string::npos == pos_from) {
        _deal_unknown();
        return;
    }
    // 判断这两个中间是否为空
    if (pos + 1 == pos_from) {
        std::cout << "未选择任何列,请检查之后重新输入!" << std::endl;
        return;
    }

    // 拿出需要查找的列
    std::string command_columns = std::string(m_command.begin() + pos + 1, m_command.begin() + pos_from);
    // 这里必须有一个空格，否则<column>就和from连在一起了，不合适
    if (' ' != command_columns.back()) {
        _deal_unknown();
        return;
    }
    // 同理，from后面也要有一个空格，这就有两种错误情况，一是from后面没东西了，二是有东西但是粘在一起了
    if (pos_from + 4 == m_command.size() or ' ' != m_command[pos_from + 4]) {
        _deal_unknown();
        return;
    }

    // 弹掉空格
    command_columns.pop_back();

    // 我们仍不允许末尾出现 ','
    if (',' == command_columns.back()) {
        std::cout << "<column>末尾不需要 ','!请检查之后重试!" << std::endl;
        return;
    }

    // 拿到正确的<column>之后，进行处理
    std::vector<std::string> show_columns;
    // 为*表示全部展示，show_column为空
    if ("*" != command_columns) {
        show_columns = my_spilt(command_columns, ',');
        for (auto& column : show_columns) {
            // 去掉多余的空格
            pop_space(column);
            // std::cout << '(' << column << ')' << std::endl;
            // 如果逗号之间的<column>中间还含有空格，命令肯定不对
            if (std::string::npos != column.find(' ')) {
                _deal_unknown();
                return;
            }
        }
    }

    // 然后读取表
    std::string command_tablename_where = std::string(m_command.begin() + pos_from + 4 + 1, m_command.end());
    std::string table_name;

    // 如果没有where，那么不允许出现空格
    size_t pos_where = command_tablename_where.find("where");
    // where不存在
    std::vector<std::string> name_val;  // 在这里提前定义where后面的语句

    if (std::string::npos == pos_where) {
        if (std::string::npos != command_tablename_where.find(' ')) {
            _deal_unknown();
            return;
        }
        // 拿到table_name
        table_name = command_tablename_where;
    }
    // 存在
    else {
        // where存在，前面必须存在空格
        if (' ' != command_tablename_where[pos_where - 1]) {
            _deal_unknown();
            return;
        }
        // 后面也必须存在
        if (pos_where + 5 == command_tablename_where.size() or ' ' != command_tablename_where[pos_where + 5]) {
            _deal_unknown();
            return;
        }
        table_name = std::string(command_tablename_where.begin(), command_tablename_where.begin() + pos_where - 1);

        // where正确了，获取where后面的命令
        std::string command_after_where = std::string(command_tablename_where.begin() + pos_where + 5 + 1, command_tablename_where.end());
        if (std::string::npos == command_after_where.find('=') or std::string::npos != command_after_where.find("==")) {  // 我怕输入 == ，这里还是判断一下
            std::cout << "您输入的where条件 " << command_after_where << " 不正确,请检查之后重新输入" << std::endl;
            return;
        }

        name_val = my_spilt(command_after_where, '=');
        for (auto& each : name_val)
            pop_space(each);
    }

    // 判断表文件是否存在
    std::string path = Order::data_prefix + m_dbname + '/' + table_name + ".dat";
    if (0 != access(path.c_str(), F_OK)) {
        std::cout << "表 " << table_name << " 不存在,请检查名称并修改!" << std::endl;
        return;
    }

    // 这时候读入table对象，因为要比对了
    table = read_table_from_file(path);

    std::cout << "表 " << table.m_table_name << " 查询结果如下: " << std::endl;

    // 显示字段名称
    // 在检测字段的时候就存储一个bool数组记录哪些列是需要显示的
    bool is_show[table.m_columns.size()] = {0};

    int where_point = -1;  // 定义where条件是判断哪一列
    for (int i = 0; i < table.m_columns.size(); ++i) {
        if (show_columns.empty() or
            show_columns.end() != std::find(show_columns.begin(), show_columns.end(), table.m_columns[i].m_column_name)) {
            std::cout << table.m_columns[i].m_column_name << ' ';

            is_show[i] = true;
            if (std::string::npos != pos_where and name_val[0] == table.m_columns[i].m_column_name)
                where_point = i;
        }
    }
    std::cout << std::endl;  // 这里需要换行刷新缓冲区，否则等命令结束后外面把标准输出重定向回去就输出到终端了

    // 显示数据
    for (auto& row : table.m_data) {
        bool flag = false;
        for (int i = 0; i < table.m_columns.size(); ++i) {
            // 没有where
            if (std::string::npos == pos_where) {
                flag = true;
                if (is_show[i])
                    std::cout << row[i] << ' ';
            }
            // 有where
            else {
                // 先满足规则条件才能进行后面的输出
                if (name_val[1] == row[where_point]) {
                    flag = true;
                    if (is_show[i])
                        std::cout << row[i] << ' ';
                }
            }
        }
        if (flag)
            std::cout << std::endl;
    }
}

// 后面几个实现我准备按照某些规则把字符串进行切割，然后进行判断，这样看会不会方便点
// delete <table> [where <cond>]
void Order::_deal_delete() {
    if (!_check_if_use())
        return;
    // TODO
}

// insert <table> values (<const-value>[, <const-value>...])
void Order::_deal_insert() {
    if (!_check_if_use())
        return;
    // TODO
}

// update <table> set <column> = <const-value> [where <cond>]
void Order::_deal_update() {
    if (!_check_if_use())
        return;
    // TODO
}

void Order::_deal_unknown() {
    std::cout << "您输入的命令不存在或者不正确,请检查之后重新输入!" << std::endl;
}
