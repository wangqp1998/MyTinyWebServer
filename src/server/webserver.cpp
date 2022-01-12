#include "../lib/webserver.h"
#include "../lib/log.h"


WebServer::WebServer()
{
    users = new http_conn[MAX_FD];
    
    //root文件夹路径
    char server_path[200];
    getcwd(server_path, 200);
    char root[6] = "/root";
    m_root = (char *)malloc(strlen(server_path) + strlen(root) + 1);
    strcpy(m_root, server_path);
    strcat(m_root, root);

    users_time = new client_data[MAX_FD];
}

WebServer::~WebServer()
{

}

void WebServer::init(int port ,bool log_write ,bool close_log ,int actor_model,int thread_num)
{
    m_port=port;
    m_log_write=log_write;
    m_close_log=close_log;
    m_actormodel=actor_model;
    m_thread_num=thread_num;
}

void WebServer::log_write()
{
    if (0 == m_close_log)
    {
        //初始化日志
        if (1 == m_log_write)
            Log::get_instance()->init("./ServerLog", m_close_log, 2000, 800000, 800);
        else
            Log::get_instance()->init("./ServerLog", m_close_log, 2000, 800000, 0);
    }
    LOG_INFO("日志初始化完成");
}

void WebServer::thread_pool()
{
    //线程池
    m_pool = new threadpool<http_conn>(0);
    LOG_INFO("线程池初始化完成");
}


void WebServer::eventListen()
{
    //创建监听套接字     iPv4    流服务
    m_listenfd = socket(PF_INET,SOCK_STREAM,0);
    assert(m_listenfd >= 0);

    int ret;
    struct sockaddr_in address;
    bzero(&address,sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(m_port);
    
    //端口复用
    int flag =1;
    setsockopt(m_listenfd,SOL_SOCKET,SO_REUSEADDR,&flag,sizeof(flag));

    //绑定地址结构到套接字描述符中
    ret = bind(m_listenfd,(struct sockaddr*)&address,sizeof(address));
    assert(ret >=0);
    
    //设置监听
    ret = listen(m_listenfd,5);
    assert(ret >=0);
    
    //创建epoll内核时间表
    m_epollfd = epoll_create(5);
    assert(m_epollfd != -1);
    //将lfd添加至树上
    utils.addfd(m_epollfd,m_listenfd);
    http_conn::m_epollfd = m_epollfd;
    
    ret = socketpair(PF_UNIX,SOCK_STREAM, 0, m_pipefd);   //用于创建一对无名的、相互连接的套接子。 
    assert(ret !=-1);
    utils.setnonblocking(m_pipefd[1]); //对文件描述符设置非阻塞
    utils.addfd(m_epollfd,m_pipefd[0]);
    utils.addsig(SIGALRM);
    utils.addsig(SIGTERM);

    alarm(TIMESLOT);

    //工具类,信号和描述符基础操作
    Utils::u_pipefd[0] = m_pipefd[0];
    Utils::u_pipefd[1] = m_pipefd[1];
    Utils::u_epollfd = m_epollfd;
    //接收网络请求
    //char client_IP[1024];
    //socklen_t clit_addr_len = sizeof(clit_addr);
    //clitfd =  accept(m_listenfd,(struct sockaddr*)&clit_addr,&clit_addr_len);
    
    //printf("client ip:%s port:%d\n", inet_ntop(AF_INET,&clit_addr.sin_addr.s_addr,client_IP,sizeof(client_IP)), ntohs(clit_addr.sin_port));

}

void WebServer::dealwithread(int sockfd)
{
     //若监测到读事件，将该事件放入请求队列
    m_pool->append(users + sockfd,0);
}

void WebServer::dealwithwrite(int sockfd)
{
    m_pool->append(users + sockfd, 1);
}

void WebServer::eventLoop()
{
    bool  stop_server = false;
    bool timeout = false;
    while(!stop_server)
    {
        int number = epoll_wait(m_epollfd,events,MAX_EVENT_NUMBER,-1);
        if(number < 0 && errno != EINTR)
        {
            LOG_ERROR("%s","epoll failure");
            break;
        }

        for(int i=0;i < number;i++)
        {
            int socket = events[i].data.fd;

            //处理新的客户端连接
            if(socket == m_listenfd)
            {
                //接收连接请求
                //printf("接收连接请求\n");
                int flag = dealclinedata();
                if(flag == false)
                    continue;
            }
            //处理信号
            else if( (socket == m_pipefd[0]) && (events[i].events & EPOLLIN) )
            {
                bool flag = dealwithsignal(timeout, stop_server);
                //printf("处理信号\n");
                if (false == flag)
                    LOG_ERROR("%s", "dealclientdata failure");
            }
            //处理客户端上接收到的数据
            else if (events[i].events & EPOLLIN)
            {
                printf("接受数据\n");
                dealwithread(socket);
            }
            else if (events[i].events & EPOLLOUT)
            {
                printf("发送数据\n");
                dealwithwrite(socket);
            }
        }
        if (timeout)
        {
            utils.timer_hander();

            LOG_INFO("%s", "timer tick");

            timeout = false;
        }
    }

}

bool WebServer::dealclinedata()
{
    struct sockaddr_in client_address;
    socklen_t clit_addr_len = sizeof(client_address);

    int connfd = accept(m_listenfd,(struct sockaddr*)&client_address,&clit_addr_len);
    if(connfd < 0)
    {
        LOG_ERROR("%s:errno is %d","accept error",errno);
        return false;
    }
    utils.addfd(m_epollfd,connfd);
    //char client_IP[1024];
    
    //printf("client ip:%s port:%d\n", inet_ntop(AF_INET,&client_address.sin_addr.s_addr,client_IP,sizeof(client_IP)), ntohs(client_address.sin_port));
    timer(connfd, client_address);
    return true;
 
}

void WebServer::timer(int connfd, struct sockaddr_in client_address)
{   
    users[connfd].init(connfd, client_address,m_close_log,m_root);

    time_t cur;  
    cur = time((time_t*) NULL);
    users_time[connfd].address=client_address;
    users_time[connfd].socker =connfd;
    until_time *timer=new until_time;
    timer ->user_data = &users_time[connfd];
    timer->cd_func = cd_func;
    timer->expire = cur +3*TIMESLOT;
    users_time[connfd].timer=timer;
    utils.m_timer_lst.add_timer(timer);
}

bool WebServer::dealwithsignal(bool& timeout, bool& stop_server)
{
    int sig;
    char signals[1024];
    int ret = recv(m_pipefd[0],signals,sizeof(signals),0);
    if(ret == -1 || ret == 0)
    {
        return false;
    }
    else
    {
        for(int i=0;i<ret;++i)
        {
            switch(signals[i])
            {
                case SIGALRM: 
                    {
                        timeout = true;
                        break;
                    }
                case SIGTERM:
                    {
                        stop_server = true;
                        break;
                    }
            }
        }
    }
    return true;
}