/**
 * @file tools.cpp
 * @brief
 * @author lzx0626 (2065666169@qq.com)
 * @version 1.0
 * @date 2023-10-25
 *
 * @copyright Copyright (c) 2023  电子科技大学
 *
 */

#include "tools.h"

/**
 * @brief 实现头文件中声明的工具函数
 */

void open_and_print(const std::string& path) {
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

std::vector<std::string> my_spilt(const std::string& str, const char& ch) {
    // 在我们的项目中，用于切割 ','
    // 例如: " name string , id int , gender string "
    size_t pos = std::string::npos;
    std::vector<std::string> ret;
    std::string sub_str = str;

    while (1) {
        pos = sub_str.find(ch);
        if (std::string::npos == pos) {
            // 把最后一个压入
            ret.push_back(sub_str);
            break;
        }

        ret.push_back(std::string(sub_str.begin(), sub_str.begin() + pos));
        sub_str = std::string(sub_str.begin() + pos + 1, sub_str.end());
    }

    return ret;
}

void pop_blank(std::string& str) {
    if (' ' == str.front())
        str.erase(str.begin());
    if (' ' == str.back())
        str.pop_back();
}

bool check_has_any(const std::string& str, const std::vector<char>& chs) {
    for (auto& ch : chs)
        if (std::string::npos != str.find(ch))
            return true;

    return false;
}

//********这两个函数为了省事，我是让chat帮我写的，我提供了存储的思路，就是write_函数里面的思路********/
void write_table_to_file(const Table& table, const std::string& path) {
    FILE* file = fopen(path.c_str(), "w");
    if (nullptr == file) {
        perror("fopen");
        exit(-1);
    }

    // 写入表名，fprintf格式化IO可以格式化写入到字符串中
    fprintf(file, "%s\n", table.m_table_name.c_str());

    // 写入列数
    size_t column_nums = table.m_columns.size();
    fwrite(&column_nums, sizeof(size_t), 1, file);
    fprintf(file, "\n");

    // 写入每列的类型和名称
    for (auto& column : table.m_columns) {
        fprintf(file, "%s\n", column.m_column_name.c_str());
        fprintf(file, "%s\n", column.m_column_type.c_str());
    }

    // 写入数据
    for (auto& row : table.m_data) {
        size_t row_size = row.size();

        fwrite(&row_size, sizeof(size_t), 1, file);
        fprintf(file, "\n");

        for (auto& cell : row)
            fprintf(file, "%s\n", cell.c_str());
    }

    fclose(file);
}

Table read_table_from_file(const std::string& path) {
    Table table;

    // 按照写入的格式读取即可
    FILE* file = fopen(path.c_str(), "r");
    if (nullptr == file) {
        perror("fopen");
        exit(-1);
    }

    char read_buf[BUFSIZ];
    // 读取表名
    if (fgets(read_buf, BUFSIZ - 1, file)) {
        table.m_table_name = read_buf;
        table.m_table_name.erase(table.m_table_name.find_last_not_of(" \n\r\t") + 1);
    }

    // 读取列数
    size_t numColumns;
    if (fread(&numColumns, sizeof(size_t), 1, file) == 1)
        fgetc(file);  // Read and discard newline character

    // 读取每列的类型和名称
    for (size_t i = 0; i < numColumns; ++i) {
        Column column;

        bzero(read_buf, BUFSIZ);
        if (fgets(read_buf, BUFSIZ - 1, file)) {
            column.m_column_name = read_buf;
            column.m_column_name.erase(column.m_column_name.find_last_not_of(" \n\r\t") + 1);
        }
        bzero(read_buf, BUFSIZ);
        if (fgets(read_buf, BUFSIZ - 1, file)) {
            column.m_column_type = read_buf;
            column.m_column_type.erase(column.m_column_type.find_last_not_of(" \n\r\t") + 1);
        }

        table.m_columns.push_back(column);
    }

    // 读取数据
    while (0 == feof(file)) {
        size_t row_size;
        if (fread(&row_size, sizeof(size_t), 1, file) != 1)
            break;

        fgetc(file);  // Read and discard newline character
        std::vector<std::string> row;
        for (size_t i = 0; i < row_size; ++i) {
            bzero(read_buf, BUFSIZ);
            if (fgets(read_buf, BUFSIZ - 1, file)) {
                std::string cell = read_buf;
                cell.erase(cell.find_last_not_of(" \n\r\t") + 1);
                row.push_back(cell);
            }
        }

        table.m_data.push_back(row);
    }

    fclose(file);

    return table;
}
