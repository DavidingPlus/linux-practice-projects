/**
 * @file tools.h
 * @brief 项目中的一些工具函数的定义和实现
 * @author lzx0626 (2065666169@qq.com)
 * @version 1.0
 * @date 2023-10-25
 *
 * @copyright Copyright (c) 2023  电子科技大学
 *
 */

#ifndef _TOOLS_H_
#define _TOOLS_H_

#include <cstring>
#include <iostream>
#include <string>
#include <vector>

/**
 * @brief 打开对应位置的文件，并且将里面的内容打印出来，定义成为extern，因为两个源文件都需要使用
 * @param  path，文件对应的目录，可能是绝对路径，也可能是相对路径
 */
void open_and_print(const std::string& path);

/**
 * @brief 给定指定的字符串，按照指定的字符进行切割，类似于python的spilt函数
 * @param  str，源字符串
 * @param  ch，分割字符
 * @return std::vector<std::string>，返回切割后的字符串列表
 */
std::vector<std::string> my_spilt(const std::string& str, const char& ch);

/**
 * @brief 弹掉字符串首尾的空格(如果存在)
 * @param  str
 */
void pop_blank(std::string& str);

/**
 * @brief 检测字符串中是否包含传入的各种字符
 * @param  str
 * @param  chs，传入的字符列表
 * @return true
 * @return false
 */
bool check_has_any(const std::string& str, const std::vector<char>& chs);

#endif
