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

#include "server_table.h"

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
 * @param  str，需要检测的字符串对象
 */
void pop_space(std::string& str);

/**
 * @brief 检测字符串中是否包含传入的各种字符
 * @param  str，需要检测的字符串对象
 * @param  chs，传入的字符列表
 * @return true
 * @return false
 */
bool check_has_any(const std::string& str, const std::vector<char>& chs);

/**
 * @brief 将Table对象的表对象按照某种方式写入文件，方便后续的读取
 * @brief 由于我们的表里面含有vector，没办法确定大小，所以新实例化的Table对象指针没办法定位终点的位置，直接读内存溢出，段错误
 * @param  table，需要写入文件的对象
 * @param  path，写入的文件路径
 */
void write_table_to_file(const Table& table, const std::string& path);

/**
 * @brief 从表文件中读取表，并且返回结构体存储的表
 * @param  path，表文件的路径
 * @return Table
 */
Table read_table_from_file(const std::string& path);

#endif
