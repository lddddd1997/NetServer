#include "EchoServer.h"
#include "Logger.h"
#include <iostream>
using namespace std;

int main()
{
    // Logger().EnableAsyncLogging("echo_server", 5 * 1024 * 1024);
    // cout << Timestamp::Now().ToFormattedString().c_str() << endl;
    EventLoop basic_loop("basic");
    EchoServer echo_server(&basic_loop, 8888, 3, 5);
    echo_server.Start();

    basic_loop.Looping();
    return 0;
}

