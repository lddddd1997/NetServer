#include <EchoServer.h>
#include <iostream>
using namespace std;

int main()
{
    EventLoop basic_loop("basic");
    EchoServer echo_server(&basic_loop, 8888, 0);
    echo_server.Start();

    basic_loop.Looping();
    return 0;
}