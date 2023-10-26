/**
 * @file server_table.h
 * @brief 存储数据库表的数据结构的类的头文件
 * @author lzx0626 (2065666169@qq.com)
 * @version 1.0
 * @date 2023-10-24
 *
 * @copyright Copyright (c) 2023  电子科技大学
 *
 */

#ifndef _SERVER_TABLE_H_
#define _SERVER_TABLE_H_

#include <iostream>
#include <string>
#include <vector>

/**
 * @brief 表中的每一列(一个字段)，包含类型和命名
 */
struct Column {
    /**
     * @brief 默认构造函数
     */
    Column() = default;

    /**
     * @brief 构造函数
     */
    Column(const std::string& column_name, const std::string& column_type)
        : m_column_name(column_name), m_column_type(column_type) {}

    /**
     * @brief 默认析构函数
     */
    ~Column() = default;

    /**
     * @brief 字段的命名
     */
    std::string m_column_name;

    /**
     * @brief 字段的类型
     */
    std::string m_column_type;
};

/**
 * @brief 存储表的结构体
 */
struct Table {
    /**
     * @brief 默认构造函数
     */
    Table() = default;

    /**
     * @brief 默认析构函数
     */
    ~Table() = default;

    /**
     * @brief 表的名字
     */
    std::string m_table_name;

    /**
     * @brief 存储所有的字段
     */
    std::vector<Column> m_columns;

    /**
     * @brief 存储所有的数据，数据包含多项，每一项又包含不同的字段
     */
    std::vector<std::vector<std::string>> m_data;
};

#endif
