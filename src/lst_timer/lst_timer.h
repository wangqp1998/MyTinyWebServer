#ifndef LST_TIMER
#define LST_TIMER

#include <time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <assert.h>
#include <sys/epoll.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include<netinet/in.h>
#include <arpa/inet.h>

class until_time;

struct client_data
{
    sockaddr_in address;
    int socker;
    until_time *timer;
};

// 定时器类
class until_time
{
public:
    until_time():prev(NULL),next(NULL){}
public:
    time_t expire;  //任务超时时间
    void (*cd_func)(client_data*);  //任务回调函数

    client_data* user_data;
    until_time* prev; //指向前一个定时器
    until_time* next; //指向下一个定时器
};
//定时器链表。它是一个升序、双向链表，且带有头节点和尾节点
class sort_timer_lst
{
public:
    sort_timer_lst():head(NULL),tail(NULL){}
    //删除其中所有的定时器
    ~sort_timer_lst();

    void add_timer(until_time* timer);
    void adjust_timer(until_time* timer);
    void del_timer(until_time* timer);
    void tick();

private:
    void add_timer(until_time* timer,until_time* lst_head); 
    
    until_time* head;
    until_time* tail;
};

class Utils
{
public:
    Utils(int timeslot=5):m_TIMESLOT(timeslot){
    };
    ~ Utils() {};

    //对文件描述符设置非阻塞
    int setnonblocking(int fd);

    //将内核事件注册为读时间
    void addfd(int epollfd, int fd);

    //信号处理函数
    static void sig_handler(int sig);

    //设置信号函数
    void addsig(int sig);

    //定时处理任务，重新定时可以不断触发SIGALRM信号
    void timer_hander();

public:
    int m_TIMESLOT;
    static int u_pipefd[2];
    static int u_epollfd;
    sort_timer_lst m_timer_lst;
};

void cd_func(client_data *user_data);
#endif