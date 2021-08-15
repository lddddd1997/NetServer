#include "HttpServer.h"
#include "Logger.h"
#include <iostream>
#include <signal.h>
using namespace std;

int main(int argc, char **argv)
{
    if(argc != 6)
    {
        std::cerr << "Input [port] [io_thread_num] [worker_thread_num] [idle_seconds] [log_switch]" << std::endl;
        exit(EXIT_FAILURE);
    }
    int port = stoi(argv[1]);
    int io_thread_num = stoi(argv[2]);
    int worker_thread_num = stoi(argv[3]);
    int idle_seconds = stoi(argv[4]);
    int log_switch = stoi(argv[5]);
    std::cout << "port: " << port << ", io_thread_num: " << io_thread_num <<
                ", worker_thread_num: " << worker_thread_num << " idle_seconds: " <<
                idle_seconds << " log_switch: " << (log_switch > 0) << std::endl;
    if(log_switch > 0)
    {
        Logger().EnableAsyncLogging("http", 100 * 1024 * 1024);
    }
    signal(SIGPIPE, SIG_IGN);
    EventLoop basic_loop("basic");
    HttpServer http_server(&basic_loop, port, io_thread_num, worker_thread_num, idle_seconds);
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

