#include "src/server/webserver.h"

int main(int argc, char *argv[])
{
    WebServer server;

    server.init();

    server.log_write();

    server.thread_pool();

    server.eventListen();

    server.eventLoop();

}