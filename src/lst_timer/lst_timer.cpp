#include "lst_timer.h"

 sort_timer_lst::~sort_timer_lst()
 {
     until_time* tmp =head;
     while (tmp)
     {
         head = tmp->next;
         delete tmp;
         tmp = head ;
     }
 }

void sort_timer_lst::add_timer(until_time* timer,until_time* lst_head)
{
    until_time* prev = lst_head;    
    until_time* tmp = prev -> next;
    while(tmp)
    {
        if(tmp->expire > timer->expire)
        {
            prev -> next = timer;
            timer -> next = tmp;
            tmp ->prev = timer;
            timer ->prev = prev;
            break;
        }
        prev = tmp;
        tmp = prev ->next;
    }
    //该timer超过链尾的节点，此时prev指向tail
    if(!tmp)
    {
        prev->next = timer;
        timer -> prev = prev;
        timer -> next = NULL;
        tail = timer;
    }
}

void sort_timer_lst::add_timer(until_time* timer)
{
    if(!timer)  //timer = 0
    {
        return;
    }
    if(!head)  //head = 0
    {
        head = tail = timer;  //头=尾=当前节点
        return;
    }
    if(timer->expire < head -> expire)  //任务超时时间比头节点的小，则作为新的头节点
    {
        timer ->next = head;
        head ->prev = timer;
        head = timer;
        return;
    }
    add_timer(timer,head);   //重载函数，升序插入合适的位置
}



void sort_timer_lst::adjust_timer(until_time* timer)
{
    if(!timer)
    {
        return;
    }
    until_time* tmp = timer ->next;
    // 若被调整的目标处在链表尾部(只考虑延长时间)，或者该定时器新的超时值仍然小于其下一个定时器的超时值，则不用调整  
    if(!tmp || (timer->expire < tmp->expire))
    {
        return ;
    }
    if(timer == head)
    {
        head = head -> next;
        head ->prev = NULL;
        timer -> next = NULL;
        add_timer(timer,head);
    }
    else
    {
        timer->prev->next = timer->next;
        timer->next->prev = timer->prev;
        add_timer(timer,timer->next);
    }
}

void sort_timer_lst::del_timer(until_time *timer)
{
    if(!timer)
    {
        return;
    }
    //链表只有一个定时器时
    if((timer == head) && (timer == tail))
    {
        delete timer;
        head = NULL;
        tail = NULL;
        return;
    }
    //删除头节点
    if(timer == head)
    {
        head = head ->next;
        head -> prev = NULL;
        delete timer;
        return;
    }
    //删除尾节点
    if(timer == tail)
    {
        tail = tail -> prev;
        tail -> next = NULL;
        delete timer;
        return ;
    }
    (timer -> next) -> prev = timer -> prev;
    (timer -> prev) -> next = timer -> next;
    delete timer;
}

void sort_timer_lst::tick()
{
    if(!head)
    {
        return;
    }
    printf("time tick\n");
    time_t cur = time(NULL);   //系统当前时间
    until_time* tmp = head;
    while(tmp)
    {
        //判断是否到期
        if(cur < tmp -> expire)
        {
            break;
        }
        //调用回调函数
        tmp->cd_func(tmp->user_data);
        //删除，并重置链表头结点
        head = tmp->next;
        if( head )
        {
            head->prev = NULL;
        }
        delete tmp;
        tmp = head;
    }
}

int Utils::setnonblocking(int fd)
{
    int old_option = fcntl(fd,F_GETFL); //取得文件描述符状态旗标
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd,F_SETFL,new_option);
    return old_option;
}

void Utils::addfd(int epollfd,int fd)
{
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET;  //ET模式
    epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&event);
    setnonblocking(fd);
}

void Utils::sig_handler(int sig)
{
    int save_errno = errno;
    int msg = sig;
    send(u_pipefd[1],(char* )&msg,1,0);   //功能是向一个已经连接的socket发送数据
    errno = save_errno;
}

void Utils::addsig(int sig)
{
    struct sigaction sa;
    memset(&sa,'\0',sizeof(sa));
    sa.sa_handler = sig_handler;    //信号处理函数
    sa.sa_flags |= SA_RESTART;      //设置信号处理的其他相关操作 如果信号中断了进程的某个系统调用，则系统自动启动该系统调用
    sigfillset(&sa.sa_mask);        //将参数set信号集初始化，然后把所有的信号加入到此信号集里即将所有的信号标志位置为1，屏蔽所有的信号
    assert(sigaction(sig,&sa,NULL) != -1);  //sigaction函数的功能是检查或修改与指定信号相关联的处理动作
}


void Utils::timer_hander()
{
    m_timer_lst.tick();
    alarm(m_TIMESLOT);
}

int Utils::u_pipefd[2] = {0};
int Utils::u_epollfd = 0;

class Utils;

void cd_func(client_data *user_data)
{
    epoll_ctl(Utils::u_epollfd, EPOLL_CTL_DEL, user_data->socker, 0);
    assert(user_data);
    close(user_data->socker);
    printf("close fd %d\n",user_data->socker);
    //http_conn::m_user_count--;
}