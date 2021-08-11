#include "HttpServer.h"
#include "Logger.h"
#include <iostream>
using namespace std;

int main()
{
    // Logger().EnableAsyncLogging("http_server", 5 * 1024 * 1024);
    EventLoop basic_loop("basic");
    HttpServer http_server(&basic_loop, 8888, 2, 2, 5);
    http_server.Start();

    basic_loop.Looping();
    return 0;
}

