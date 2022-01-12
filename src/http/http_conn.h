#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/uio.h>
#include <map>

#include "../locker/locker.h"
//#include "../CGImysql/sql_connection_pool.h"
#include "../lst_timer/lst_timer.h"
#include "../log/log.h"

class http_conn
{
public:
    static const int FILENAME_LEN = 200;          //文件名的最大长度
    static const int READ_BUFFER_SIZE = 2048;   //读缓存的大小
    static const int WRITE_BUFFER_SIZE = 1024; //写缓存的大小
    enum METHOD  //HTTP请求方法
    {
        GET = 0,
        POST,
        HEAD,
        PUT,
        DELETE,
        TRACE,
        OPTIONS,
        CONNECT,
        PATH
    };
    enum HTTP_CODE   //服务器处理HTTP请求的结果
    {
        NO_REQUEST,          //请求不完整，需要继续读取客户数据
        GET_REQUEST,         //获得了一个完整的客户请求
        BAD_REQUEST,         //客户请求有语法错误
        NO_RESOURCE,         //
        FORBIDDEN_REQUEST,   //表示客户对资源没有足够的访问权限
        FILE_REQUEST,
        INTERNAL_ERROR,      //服务器内部错误
        CLOSED_CONNECTION    //表示已经关闭连接了
    };
    enum CHECK_STATE  //解析客户请求时，主状态机所处的状态
    {
        CHECK_STATE_REQUESTLINE = 0,   //正在分析请求行
        CHECK_STATE_HEADER,            //正在分析头部字段
        CHECK_STATE_CONTENT            //正在分析内容
    };
    enum LINE_STATUS   //状态机的三种可能状态
    {
        LINE_OK = 0,    //读取到一个完整的行
        LINE_BAD,       //行出错
        LINE_OPEN       //行数据不完整
    };


public:
    http_conn() {}
    ~http_conn() {}

public:
    void init(int sockfd, const sockaddr_in &addr,int close_log, char *root);

    bool read_once();
    bool write();
    void process();

private:
    void init();
    void close_conn(bool real_close = true);
    HTTP_CODE do_request();

    HTTP_CODE process_read();
    LINE_STATUS parse_line();
    char* get_line(){return m_read_buf + m_start_line;}
    HTTP_CODE parse_request_line(char *text);
    HTTP_CODE parse_headers(char *text);
    HTTP_CODE parse_content(char *text);

    bool process_write(HTTP_CODE ret);
    bool add_response(const char *format, ...);
    bool add_status_line(int status, const char *title);
    bool add_headers(int content_len);
    bool add_content_length(int content_len);
    bool add_content_type();
    bool add_linger();
    bool add_blank_line();
    bool add_content(const char *content);

    void unmap();
public:
    static int m_epollfd; 
    int m_state;  //读为0, 写为1

private:
    
    int m_sockfd;
    sockaddr_in m_address;

    char m_read_buf[READ_BUFFER_SIZE];
    int m_read_idx;                     //标识读缓冲区已经读入数据的下一个位置
    int m_checked_idx;                  //正在解析的字符在读缓冲区的位置
    int m_start_line;                   //当前解析行的起始位置

    char *doc_root;                     //根目录
    char m_real_file[FILENAME_LEN];

    char m_write_buf[WRITE_BUFFER_SIZE];
    int m_write_idx;
    int bytes_to_send;
     int bytes_have_send;


    char *m_file_address;                //客户请求的目标文件被mmap到内存中的起始位置
    struct stat m_file_stat;             //目标文件状态
    struct iovec m_iv[2];                //写内存块
    int m_iv_count;
    

    char *m_url;                         //文件名
    char *m_version;                     //HTTP协议版本号
    char *m_host;                        //主机名
    int m_content_length;                //消息体的长度
    bool m_linger;                       //HTTP是否要求保持连接
    METHOD m_method;                     //请求方法
 
    CHECK_STATE m_check_state;          //主状态机当前所处的状态

     

    int m_close_log;
};

#endif
