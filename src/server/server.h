#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <unordered_map>
#include <fcntl.h>       // fcntl()
#include <unistd.h>      // close()
#include <assert.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <memory>

#include "epoller.h"
#include "../log/log.h"
//#include "../timer/heaptimer.h"
//#include "../pool/sqlconnpool.h"
//#include "../pool/threadpool.h"
//#include "../pool/sqlconnRAII.h"
//#include "../http/httpconn.h"

class WebServer
{
public:

    WebServer(
        int port, int trigMode, int timeoutMS, bool OptLinger,     //端口 ET模式 timeoutMS 优雅退出
        int sqlPort, const char* sqlUser, const  char* sqlPwd,     //Mysql配置
        const char* dbName, int connPoolNum, int threadNum,        //连接池数量 线程数量 
        bool openLog, int logLevel, int logQueSize);               //日志开关 日志等级 日志异步队列容量

    ~WebServer();
    void Start();

private:
    bool InitSocket_();    
   

private:

    static const int MAX_FD = 65536;

    static int SetFdNonblock(int fd);

    int port_;
    bool openLinger_;
    int timeoutMS_;  /* 毫秒MS */
    bool isClose_;
    int listenFd_;
    char* srcDir_;

    int openLog_;
    
    uint32_t listenEvent_;
    uint32_t connEvent_;
   
    //std::unique_ptr<HeapTimer> timer_;
    //std::unique_ptr<ThreadPool> threadpool_;
    std::unique_ptr<Epoller> epoller_;    //智能指针
    //std::unordered_map<int, HttpConn> users_;
};






#endif