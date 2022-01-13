#ifndef WEBSERVER_H
#define WEBSERVER_H


#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/epoll.h>
#include <string.h>
#include <signal.h>
#include <memory>

#include "../log/log.h"
#include "../lst_timer/lst_timer.h"
#include "../threadpool/threadpool.h"
#include "../http/http_conn.h"
#include "epoller.h"

const int MAX_FD = 65536;  //最大文件描述符
const int MAX_EVENT_NUMBER = 10000; //最大事件数
const int TIMESLOT = 5;             //最小超时单位

class WebServer
{
public:

    WebServer();
    ~WebServer();

    void init(int port = 9600,bool log_write = 0,bool close_log = 0,int m_actormodel =0,int m_thread_num =8000);
    void log_write();
    void thread_pool();

    void eventListen();
    void eventLoop();
    bool dealclinedata();
    void timer(int connfd,struct sockaddr_in client_address);
    bool dealwithsignal(bool& timeout, bool& stop_server);
    void dealwithread(int sockfd);
    void dealwithwrite(int sockfd);
public:
    int m_port;
    char *m_root;
    
    int m_pipefd[2];
    int m_epollfd;

    int m_listenfd;   //监听套接字
    int clitfd;       //客户端套接字

    bool m_log_write;
    bool m_close_log;

    //epoll相关
    epoll_event events[MAX_EVENT_NUMBER];

    //定时器相关
    client_data *users_time;
    Utils utils;


    int m_actormodel;
    int m_thread_num;
    threadpool<http_conn> *m_pool;

    http_conn *users;
    //int m_CONNTrigmode;
};


#endif
