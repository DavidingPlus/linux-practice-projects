/**
 * @file http_conn.h
 * @brief HTTP任务类文件
 * @author 刘治学
 */

#ifndef _HTTP_CONN_H_
#define _HTTP_CONN_H_

#include <arpa/inet.h>
#include <fcntl.h>
#include <stdarg.h>
#include <sys/epoll.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

/**
 * @brief 定义读缓冲区的大小，定义为宏
 */
#define READ_BUFFER_SIZE 2048

/**
 * @brief 定义写缓冲区的大小
 */
#define WRITE_BUFFER_SIZE 1024

/**
 * @brief 文件名的最大长度
 */
#define FILENAME_LEN 200

/**
 * @brief HTTP任务类
 */
class Http_Conn {
public:
    // 存放有限状态机的需要用到的一些信息
    /**
     * @brief HTTP请求方法，这里只支持GET
     */
    enum METHOD { GET = 0,
                  POST,
                  HEAD,
                  PUT,
                  DELETE,
                  TRACE,
                  OPTIONS,
                  CONNECT };

    /**
     * @brief   解析客户端请求时，主状态机的状态
                CHECK_STATE_REQUESTLINE   :     当前正在分析请求行
                CHECK_STATE_HEADER        :     当前正在分析头部字段
                CHECK_STATE_CONTENT       :     当前正在解析请求体
    */
    enum CHECK_STATE { CHECK_STATE_REQUESTLINE = 0,
                       CHECK_STATE_HEADER,
                       CHECK_STATE_CONTENT };

    /**
     * @brief   服务器处理HTTP请求的可能结果，报文解析的结果
                NO_REQUEST          :       请求不完整，需要继续读取客户数据
                GET_REQUEST         :       表示获得了一个完成的客户请求
                BAD_REQUEST         :       表示客户请求语法错误
                NO_RESOURCE         :       表示服务器没有资源
                FORBIDDEN_REQUEST   :       表示客户对资源没有足够的访问权限
                FILE_REQUEST        :       文件请求,获取文件成功
                INTERNAL_ERROR      :       表示服务器内部错误
                CLOSED_CONNECTION   :       表示客户端已经关闭连接了
    */
    enum HTTP_CODE { NO_REQUEST = 0,
                     GET_REQUEST,
                     BAD_REQUEST,
                     NO_RESOURCE,
                     FORBIDDEN_REQUEST,
                     FILE_REQUEST,
                     INTERNAL_ERROR,
                     CLOSED_CONNECTION };

    /**
    * @brief    从状态机的三种可能状态，即行的读取状态，分别表示
                LINE_OK   :   读取到一个完整的行
                LINE_BAD  :   行出错
                LINE_OPEN :   行数据尚且不完整
    */
    enum LINE_STATUS { LINE_OK = 0,
                       LINE_BAD,
                       LINE_OPEN };

public:
    /**
     * @brief 重写构造函数
     */
    Http_Conn();

    /**
     * @brief 重写析构函数
     */
    ~Http_Conn();

    /**
     * @brief 初始化连接的相关信息，不给参数就是重载版本
     * @param socket_fd，连接的用于通信的文件描述符
     * @param addr，客户端的socket地址信息
     */
    void __init__(const int& socket_fd, const sockaddr_in& addr);

private:
    /**
     * @brief 初始化连接和状态机的相关信息
     */
    void __init__();

    /**
     * @brief 解析HTTP请求行，获取请求方法，目标URL，HTTP版本
     * @param text，字符串报文
     * @return HTTP_CODE类型，返回解析后的状态
     */
    HTTP_CODE parse_request_line(char* text);

    /**
     * @brief 解析HTTP请求的一个头部信息
     * @param text，字符串报文
     * @return HTTP_CODE类型，返回解析后的状态
     */
    HTTP_CODE parse_headers(char* text);

    /**
     * @brief 解析请求体;我们没有真正解析HTTP请求的消息体，只是判断它是否被完整的读入了
     * @param text，字符串报文
     * @return HTTP_CODE类型，返回解析后的状态
     */
    HTTP_CODE parse_content(char* text);

    /**
     * @brief 解析一行，判断依据是 \r\n
     * @return LINE_STATUS类型，返回解析后的状态
     */
    LINE_STATUS parse_line();

    /**
     * @brief 获取某一行数据的起始地址
     * @return char*，表示地址
     */
    char* get_line_address();

    /**
     * @brief 当得到一个完整，正确的HTTP请求的时候，我们就分析目标文件的属性
     *        如果目标文件存在，并且对所有用户可读，且不是目录，
     *        则使用mmap将其映射到内存地址 _file_address 处，并告诉调用者获取文件成功
     */
    HTTP_CODE do_request();

    /**
     * @brief 解析HTTP请求，主状态机，解析请求
     * @return HTTP_CODE类型，返回解析后的状态
     */
    HTTP_CODE process_read();

    /**
     * @brief 根据服务器处理HTTP请求的结果，决定返回给客户端的内容
     * @param ret，HTTP_CODE类型解析结果
     * @return bool类型，true表示成功
     */
    bool process_write(HTTP_CODE ret);

    /**
     * @brief 释放创建的内存映射，进行munmap操作
     */
    void unmap();

    /**
     * @brief 下面一组函数被创建来被process_write()调用来填充HTTP应答
     */
    bool add_response(const char* format, ...);
    bool add_content(const char* content);
    bool add_status_line(int status, const char* title);
    bool add_headers(int content_length);
    bool add_content_length(int content_length);
    bool add_stay_connection();
    bool add_blank_line();

public:
    /**
     * @brief 用来关闭连接
     */
    void close_conn();

    /**
     * @brief 处理客户端请求，有线程池中的工作线程调用，处理HTTP请求的入口函数
     */
    void process();

    /**
     * @brief 非阻塞的一次性读出所有数据
     * @return bool类型
     */
    bool _read();

    /**
     * @brief 非阻塞的写数据
     * @return bool类型
     */
    bool _write();

public:
    /**
     * @brief 所有的socket上的事件都被注册到同一个epoll对象中，所以是静态
     */
    static int _epollfd;

    /**
     * @brief 统计用户的数量，整个程序只有一份
     */
    static int _user_count;

private:
    /**
     * @brief 网站资源在系统中的根路径
     */
    static const char* _doc_root;

    /**
     * @brief 定义HTTP响应的一些状态信息
     */
    // 定义HTTP响应的一些状态信息
    static const char* _ok_200_title;
    static const char* _error_400_title;
    static const char* _error_400_form;
    static const char* _error_403_title;
    static const char* _error_403_form;
    static const char* _error_404_title;
    static const char* _error_404_form;
    static const char* _error_500_title;
    static const char* _error_500_form;

private:
    /**
     * @brief 该HTTP连接的socket套接字
     */
    int _socket_fd;

    /**
     * @brief 该HTTP通信是的socket地址
     */
    struct sockaddr_in _addr;

    /**
     * @brief 读缓冲区
     */
    char _read_buf[READ_BUFFER_SIZE];

    /**
     * @brief 标识读缓冲区已经读入的客户端数据的最后一个数据的写一个字节，下一次从这里开始读
     */
    int _read_index;

    /**
     * @brief 当前正在读取的字符在读缓冲区的位置
     */
    int _checked_index;

    /**
     * @brief 当前正在解析的行的起始位置
     */
    int _start_line;

    /**
     * @brief 主状态机当前所处的状态
     */
    CHECK_STATE _check_state;

    /**
     * @brief 请求目标文件的URL(文件名)
     */
    char* _url;

    /**
     * @brief 协议版本，我们只支持HTTP1.1
     */
    char* _version;

    /**
     * @brief 请求方法
     */
    METHOD _method;

    /**
     * @brief 主机名
     */
    char* _host;

    /**
     * @brief HTTP请求是否要保持连接
     */
    bool _stay_connection;

    /**
     * @brief HTTP请求的消息总长度
     */
    int _content_length;

    /**
     * @brief 客户请求的目标文件的完整路径，其内容等于 _doc_root + _url, _doc_root是网站根目录
     */
    char _real_file[FILENAME_LEN];

    /**
     * @brief 目标文件的状态。通过它我们可以判断文件是否存在、是否为目录、是否可读，并获取文件大小等信息
     */
    struct stat _file_stat;

    /**
     * @brief 客户请求的目标文件被mmap到内存中的起始位置
     */
    char* _file_address;

    /**
     * @brief 我们将采用writev来执行写操作，所以定义下面两个成员，其中m_iv_count表示被写内存块的数量。
     */
    struct iovec _iv[2];
    int _iv_count;

    /**
     * @brief 写缓冲区
     */

    char _write_buf[WRITE_BUFFER_SIZE];

    /**
     * @brief 写缓冲区的字节数
     */
    int _write_index;

    /**
     * @brief 将要发送的数据的字节数
     */
    int _bytes_to_send;

    /**
     * @brief 已经发送的字节数
     */
    int _bytes_have_send;
};

#endif
