#include "HttpServer.h"
#include "Logger.h"
#include <iostream>
using namespace std;

int main()
{
    EventLoop basic_loop("basic");
    HttpServer http_server(&basic_loop, 8888, 1, 0, 0);
    http_server.Start();

    try
    {
        basic_loop.Looping();
    }
    catch(const std::exception& ex)
    {
        std::cerr << "exception caught int BasicLoopThread::RunInThread at: " << std::this_thread::get_id() << std::endl;
        std::cerr << "reason: " << ex.what() << std::endl;
        exit(EXIT_FAILURE);
    }
    return 0;
}

