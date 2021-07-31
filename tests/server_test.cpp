#include <iostream>
#include <TcpServer.h>
#include <signal.h>
using namespace std;

// g++ server_test.cpp -o server -I ../ -pthread ../*.o

using TcpConnectionSPtr = std::shared_ptr<TcpConnection>;
int main()
{
    // signal(SIGPIPE, SIG_IGN);
    EventLoop basic_loop;
    TcpServer TcpServer(&basic_loop, 8888, 0);
    TcpServer.SetMessageCallback([](const TcpConnectionSPtr& conn, std::string& s){
        std::string msg;
        msg.swap(s);
        cout << "receive massage from, client address = " << inet_ntoa(conn->LocalAddress().sin_addr) << 
        ":" << ntohs(conn->LocalAddress().sin_port) << endl << ">> " << msg << endl;
        conn->Send(msg);
    });
    TcpServer.SetWriteCompleteCallback([](const TcpConnectionSPtr& conn){
        // cout << "-----------------TcpServer write complete callback" << endl;
    });
    TcpServer.SetConnectionCallback([](const TcpConnectionSPtr& conn){
        std::cout << "New client connection, address = " << inet_ntoa(conn->LocalAddress().sin_addr) << 
        ":" << ntohs(conn->LocalAddress().sin_port) << std::endl;
        // cout << "-----------------TcpServer connection callback" << endl;
    });
    TcpServer.SetCloseCallback([](const TcpConnectionSPtr& conn){
        std::cout << "client closed, address = " << inet_ntoa(conn->LocalAddress().sin_addr) << 
        ":" << ntohs(conn->LocalAddress().sin_port) << std::endl;
        // cout << "-----------------TcpServer close callback" << endl;
    });
    TcpServer.SetErrorCallback([](const TcpConnectionSPtr& conn){
        cout << "-----------------TcpServer error callback" << endl;
    });
    TcpServer.Start();
    cout << "The basic thread is running, thread id = " << basic_loop.ThreadId() << endl;

    basic_loop.Looping();
    return 0;
}
