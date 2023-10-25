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
