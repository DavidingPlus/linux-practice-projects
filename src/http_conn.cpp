/**
 * @file http_conn.h
 * @brief HTTP任务类文件
 * @author 刘治学
 */

#include "http_conn.h"

//-------------------------------------------------------------
// 类外初始化初始化静态成员，不能在类内成员函数中初始化，在main函数中修改
int Http_Conn::_epollfd = -1;
int Http_Conn::_user_count = 0;
const char* Http_Conn::_doc_root = "/home/lzx0626/DavidingPlus/Projects/res";

const char* Http_Conn::_ok_200_title = "OK";
const char* Http_Conn::_error_400_title = "Bad Request";
const char* Http_Conn::_error_400_form = "Your request has bad syntax or is inherently impossible to satisfy.\n";
const char* Http_Conn::_error_403_title = "Forbidden";
const char* Http_Conn::_error_403_form = "You do not have permission to get file from this server.\n";
const char* Http_Conn::_error_404_title = "Not Found";
const char* Http_Conn::_error_404_form = "The requested file was not found on this server.\n";
const char* Http_Conn::_error_500_title = "Internal Error";
const char* Http_Conn::_error_500_form = "There was an unusual problem serving the requested file.\n";

//-------------------------------------------------------------
// 实现main.cpp中定义的 extern 的函数

/**
 * @brief 辅助函数，设置文件描述符为非阻塞
 * @param fd，要修改性质的文件描述符
 */
void set_fd_NonBlocking(const int& fd) {
    // 获取当前的文件描述符标志
    int _flag = fcntl(fd, F_GETFL);
    _flag |= O_NONBLOCK;
    // 设置flag
    fcntl(fd, F_SETFL, _flag);
}

// 向epoll中添加需要监听的文件描述符
void add_fd(int epollfd, int fd, bool one_shot) {
    struct epoll_event event;
    event.data.fd = fd;
    // 设置为水平触发，后续可以自己修改
    event.events = EPOLLIN | EPOLLRDHUP | EPOLLET;  // EPOLLRDHUP可以在对端断开连接的时候，这个事件可以检测到，就不用走上层的read()返回值了

    if (one_shot)
        // 给事件加上EPOLLONESHOT保证只能被一个线程操作；对于监听的文件描述符可以不添加，其他的还是要加上，所以这里是给了个参数判断
        event.events |= EPOLLONESHOT;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);

    // 设置文件描述符为非阻塞，否则读的时候阻塞在那里出问题
    set_fd_NonBlocking(fd);
}

// 将文件描述符从epoll实例中删除
void remove_fd(int epollfd, int fd) {
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, nullptr);
    close(fd);
}

// 修改epoll中的文件描述符，传入事件
void modify_fd(int epollfd, int fd, int ev) {
    epoll_event event;
    event.data.fd = fd;
    // 注意重置socket对应的 EPOLLONESHOT 事件，以确保下一次可读时EPOLLIN事件能被触发，否则触发不了
    event.events = ev | EPOLLONESHOT | EPOLLRDHUP | EPOLLET;

    epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
}

//----------------------------------------------------------------
Http_Conn::Http_Conn() {
    __init__();
}

Http_Conn::~Http_Conn() {
}

void Http_Conn::__init__(const int& socket_fd, const sockaddr_in& addr) {
    _socket_fd = socket_fd;
    _addr = addr;

    // 新的客户端连接设置端口复用
    int _optval = 1;
    setsockopt(_socket_fd, SOL_SOCKET, SO_REUSEPORT, &_optval, sizeof(_optval));

    // 添加到epoll对象中
    add_fd(_epollfd, _socket_fd, true);
    // 总用户数加1
    ++_user_count;
}

void Http_Conn::__init__() {
    // 连接相关信息
    _socket_fd = -1;
    bzero(&_addr, sizeof(_addr));
    bzero(&_read_buf, sizeof(_read_buf));

    // 状态机的相关信息
    _check_state = CHECK_STATE_REQUESTLINE;  // 初始化状态为解析请求首行
    _checked_index = 0;
    _start_line = 0;
    _read_index = 0;

    // 初始化HTTP报文的相关信息
    _url = nullptr;
    _version = nullptr;
    _method = GET;
    _host = nullptr;
    _stay_connection = false;
    _content_length = 0;
    bzero(_real_file, sizeof(_real_file));
    bzero(&_file_stat, sizeof(_file_stat));
    _file_address = nullptr;
    bzero(&_iv, sizeof(_iv));
    _iv_count = 0;
    bzero(_write_buf, sizeof(_write_buf));
    _write_index = 0;
    _bytes_to_send = 0;
    _bytes_have_send = 0;
}

Http_Conn::HTTP_CODE Http_Conn::parse_request_line(char* text) {
    // GET /index.html HTTP/1.1
    _url = strpbrk(text, " \t");  // 这个函数的作用是检测第二个字符串的所有字符哪个是第一个在第一个字符串中出现的
    if (!_url)
        return BAD_REQUEST;

    *(_url++) = '\0';  // 此时_url指向'\0'后面一个字符

    // GET\0/index.html HTTP/1.1
    char* method = text;
    if (0 == strcasecmp(method, "GET"))  // 这个函数是忽略大小写的字符串比较
        _method = GET;
    else
        return BAD_REQUEST;

    // /index.html HTTP/1.1
    _version = strpbrk(_url, " \t");
    if (!_version)
        return BAD_REQUEST;
    // /index.html\0HTTP/1.1
    *(_version++) = '\0';

    if (0 != strcasecmp(_version, "HTTP/1.1"))
        return BAD_REQUEST;

    // 有可能url长这样：http://192.168.1.1:10000/index.html
    if (0 == strncasecmp(_url, "http://", 7)) {
        _url += 7;
        // 这个函数可以查找字符串中第一次出现某个字符的位置
        _url = strchr(_url, '/');  // /index.html
    }

    if (!_url || _url[0] != '/')
        return BAD_REQUEST;

    // 主状态机检查状态变为检查请求头
    _check_state = CHECK_STATE_HEADER;

    return NO_REQUEST;
}

Http_Conn::HTTP_CODE Http_Conn::parse_headers(char* text) {
    // 遇到空行，表示头部字段解析完毕
    if ('\0' == text[0]) {
        // 如果HTTP请求有消息体，则还需要读取m_content_length字节的消息体，
        // 状态机转移到CHECK_STATE_CONTENT状态
        if (0 != _content_length) {
            _check_state = CHECK_STATE_CONTENT;
            return NO_REQUEST;
        }
        // 否则说明我们已经得到了一个完整的HTTP请求
        return GET_REQUEST;
    } else if (0 == strncasecmp(text, "Connection:", 11)) {
        // 处理Connection 头部字段  Connection: keep-alive
        text += 11;
        text += strspn(text, " \t");
        if (0 == strcasecmp(text, "keep-alive"))
            _stay_connection = true;
    } else if (0 == strncasecmp(text, "Content-Length:", 15)) {
        // 处理Content-Length头部字段
        text += 15;
        text += strspn(text, " \t");
        _content_length = atol(text);
    } else if (0 == strncasecmp(text, "Host:", 5)) {
        // 处理Host头部字段
        text += 5;
        text += strspn(text, " \t");
        _host = text;
    } else
        printf("oop! unknow header %s\n", text);

    return NO_REQUEST;
}

Http_Conn::HTTP_CODE Http_Conn::parse_content(char* text) {
    if (_read_index >= (_content_length + _checked_index)) {
        text[_content_length] = '\0';
        return GET_REQUEST;
    }
    return NO_REQUEST;
}

Http_Conn::LINE_STATUS Http_Conn::parse_line() {
    char temp;

    for (; _checked_index < _read_index; ++_checked_index) {
        // 获得当前分析的字
        temp = _read_buf[_checked_index];
        // 如果当前的字节是\r，即回车符，则说明可能读取到一个完整的行
        if ('\r' == temp) {
            // 如果\r字符碰巧是目前buffer中的最后一个已经被读入的客户数据，那么这次分析没有读取到一个完整的行，需要继续读取数据
            if (_checked_index + 1 == _read_index)
                return LINE_OPEN;

            // 表示读到了一个完整的行
            else if ('\n' == _read_buf[_checked_index + 1]) {
                _read_buf[_checked_index++] = '\0';
                _read_buf[_checked_index++] = '\0';
                return LINE_OK;
            }
            // 以上都不是，就是存在语法错误
            return LINE_BAD;

        }
        // 当前字符为\n也有可能是到了一行的情况
        else if ('\n' == temp) {
            // 因为\r\n一起用，还得判断
            if (_checked_index > 1 && '\r' == _read_buf[_checked_index - 1]) {
                _read_buf[_checked_index - 1] = '\0';
                _read_buf[_checked_index++] = '\0';
                return LINE_OK;
            }
            return LINE_BAD;
        }
    }
    // 如果所有的字符都读完了还没有遇到
    return LINE_OPEN;
}

char* Http_Conn::get_line_address() {
    return _read_buf + _start_line;
}

Http_Conn::HTTP_CODE Http_Conn::do_request() {
    // /home/lzx0626/webserver/res
    strcpy(_real_file, _doc_root);
    int len = strlen(_doc_root);
    strncpy(_real_file + len, _url, FILENAME_LEN - len - 1);
    // 获取m_real_file文件的相关的状态信息，-1失败，0成功
    if (stat(_real_file, &_file_stat) < 0)
        return NO_RESOURCE;

    // 判断访问权限
    if (!(_file_stat.st_mode & S_IROTH))
        return FORBIDDEN_REQUEST;

    // 判断是否是目录
    if (S_ISDIR(_file_stat.st_mode))
        return BAD_REQUEST;

    // 以只读方式打开文件
    int fd = open(_real_file, O_RDONLY);
    // 创建内存映射
    _file_address = (char*)mmap(0, _file_stat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);

    return FILE_REQUEST;
}

Http_Conn::HTTP_CODE Http_Conn::process_read() {
    // 定义初始状态
    LINE_STATUS line_status = LINE_OK;
    HTTP_CODE ret = NO_REQUEST;

    char* text = 0;

    while ((LINE_OK == (line_status = parse_line())) ||
           ((CHECK_STATE_CONTENT == _check_state) && (LINE_OK == line_status))) {
        // 解析到了一行完整到的数据，或者解析到了请求体也是完整的数据
        // 获取一行数据
        text = get_line_address();

        _start_line = _checked_index;
        printf("%s\n", text);

        switch (_check_state) {
        case CHECK_STATE_REQUESTLINE: {
            ret = parse_request_line(text);
            if (BAD_REQUEST == ret)
                return BAD_REQUEST;
            break;
        }
        case CHECK_STATE_HEADER: {
            ret = parse_headers(text);
            if (BAD_REQUEST == ret)
                return BAD_REQUEST;
            else if (GET_REQUEST == ret)
                return do_request();
            break;
        }
        case CHECK_STATE_CONTENT: {
            ret = parse_content(text);
            if (ret == GET_REQUEST)
                return do_request();
            line_status = LINE_OPEN;
            break;
        }
        default:
            return INTERNAL_ERROR;
            break;
        }
    }
    return NO_REQUEST;
}

bool Http_Conn::process_write(HTTP_CODE ret) {
    switch (ret) {
    case INTERNAL_ERROR: {
        add_status_line(500, _error_500_title);
        add_headers(strlen(_error_500_form));
        if (!add_content(_error_500_form))
            return false;

        break;
    }

    case BAD_REQUEST: {
        add_status_line(400, _error_400_title);
        add_headers(strlen(_error_400_form));
        if (!add_content(_error_400_form))
            return false;

        break;
    }

    case NO_RESOURCE: {
        add_status_line(404, _error_404_title);
        add_headers(strlen(_error_404_form));
        if (!add_content(_error_404_form))
            return false;

        break;
    }

    case FORBIDDEN_REQUEST: {
        add_status_line(403, _error_403_title);
        add_headers(strlen(_error_403_form));
        if (!add_content(_error_403_form)) {
            return false;
        }
        break;
    }

    case FILE_REQUEST: {
        add_status_line(200, _ok_200_title);
        if (0 != _file_stat.st_size) {
            add_headers(_file_stat.st_size);
            _iv[0].iov_base = _write_buf;
            _iv[0].iov_len = _write_index;
            _iv[1].iov_base = _file_address;
            _iv[1].iov_len = _file_stat.st_size;
            _iv_count = 2;

            _bytes_to_send = _write_index + _file_stat.st_size;

            return true;
        } else {
            const char* ok_string = "<html><body></body></html>";
            add_headers(strlen(ok_string));
            if (!add_content(ok_string)) {
                return false;
            }
        }
        break;
    }

    default:
        return false;
    }

    _iv[0].iov_base = _write_buf;
    _iv[0].iov_len = _write_index;
    _iv_count = 1;
    _bytes_to_send = _write_index;

    return true;
}

void Http_Conn::unmap() {
    if (_file_address) {
        munmap(_file_address, sizeof(_file_address));
        _file_address = nullptr;
    }
}

//---------------------------------------------------------------
// 往写缓冲中写入待发送的数据
bool Http_Conn::add_response(const char* format, ...) {
    if (_write_index >= WRITE_BUFFER_SIZE)
        return false;

    va_list arg_list;
    va_start(arg_list, format);
    int len = vsnprintf(_write_buf + _write_index,
                        WRITE_BUFFER_SIZE - 1 - _write_index, format, arg_list);
    if (len >= (WRITE_BUFFER_SIZE - 1 - _write_index))
        return false;

    _write_index += len;
    va_end(arg_list);
    return true;
}

bool Http_Conn::add_status_line(int status, const char* title) {
    return add_response("%s %d %s\r\n", "HTTP/1.1", status, title);
}

bool Http_Conn::add_headers(int content_len) {
    add_content_length(content_len);
    add_stay_connection();
    add_blank_line();
    return true;
}

bool Http_Conn::add_content_length(int content_len) {
    return add_response("Content-Length: %d\r\n", content_len);
}

bool Http_Conn::add_stay_connection() {
    return add_response("Connection: %s\r\n", (true == _stay_connection) ? "keep-alive" : "close");
}

bool Http_Conn::add_blank_line() {
    return add_response("%s", "\r\n");
}

bool Http_Conn::add_content(const char* content) {
    return add_response("%s", content);
}

//---------------------------------------------------------------
void Http_Conn::close_conn() {
    if (-1 != _socket_fd) {
        remove_fd(_epollfd, _socket_fd);
        __init__();  // 将类内的属性初始化
        --_user_count;
    }
}

void Http_Conn::process() {
    // 解析HTTP请求
    HTTP_CODE read_ret = process_read();
    if (NO_REQUEST == read_ret) {  // 请求不完整，需要继续读取客户数据
        modify_fd(_epollfd, _socket_fd, EPOLLIN);
        return;
    }

    // 生成响应
    bool write_ret = process_write(read_ret);
    if (!write_ret)
        close_conn();
    modify_fd(_epollfd, _socket_fd, EPOLLOUT);
}

bool Http_Conn::_read() {
    // 循环读取客户数据
    if (_read_index >= READ_BUFFER_SIZE)  // 缓冲区已经满了，不读了，下一次读
        return false;

    // 读取到的字节数
    int bytes_read = 0;
    while (1) {
        // 从上一次读完的地方开始读，注意里面的各种参数
        bytes_read = recv(_socket_fd, _read_buf + _read_index,
                          READ_BUFFER_SIZE - _read_index, 0);
        if (-1 == bytes_read) {
            if (errno == EAGAIN || errno == EWOULDBLOCK)  // 这个错误表示没有错误了
                break;
            return false;
        } else if (0 == bytes_read)  // 对方关闭连接
            return false;
        _read_index += bytes_read;
    }

    return true;
}

bool Http_Conn::_write() {
    int temp = 0;

    if (_bytes_to_send == 0) {
        // 将要发送的字节为0，这一次响应结束。
        modify_fd(_epollfd, _socket_fd, EPOLLIN);
        __init__();
        return true;
    }

    while (1) {
        // 分散写
        temp = writev(_socket_fd, _iv, _iv_count);
        if (temp < 0) {
            // 如果TCP写缓冲没有空间，则等待下一轮EPOLLOUT事件，虽然在此期间，
            // 服务器无法立即接收到同一客户的下一个请求，但可以保证连接的完整性。
            if (errno == EAGAIN) {
                modify_fd(_epollfd, _socket_fd, EPOLLOUT);
                return true;
            }
            unmap();
            return false;
        }

        _bytes_have_send += temp;
        _bytes_to_send -= temp;

        if (_bytes_have_send >= _iv[0].iov_len) {
            _iv[0].iov_len = 0;
            _iv[1].iov_base = _file_address + (_bytes_have_send - _write_index);
            _iv[1].iov_len = _bytes_to_send;
        } else {
            _iv[0].iov_base = _write_buf + _bytes_have_send;
            _iv[0].iov_len = _iv[0].iov_len - _bytes_have_send;
        }

        if (_bytes_to_send <= 0) {
            // 没有数据要发送了
            unmap();
            modify_fd(_epollfd, _socket_fd, EPOLLIN);

            if (_stay_connection) {
                __init__();
                return true;
            } else
                return false;
        }
    }
}
