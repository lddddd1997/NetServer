#include <iostream>
#include <TcpServer.h>
#include <signal.h>
using namespace std;

// g++ server_test.cpp -o server -I ../ -pthread ../*.o

using TcpConnectionSPtr = std::shared_ptr<TcpConnection>;
int main()
{
    // signal(SIGPIPE, SIG_IGN);
    EventLoop base_loop;
    TcpServer TcpServer(&base_loop, 8888, 0);
    TcpServer.SetMessageCallback([](const TcpConnectionSPtr& conn, std::string& s){
        std::string msg;
        msg.swap(s);
        cout << msg << endl;
        conn->Send(msg);
    });
    TcpServer.SetWriteCompleteCallback([](const TcpConnectionSPtr&){
        cout << "TcpServer write complete callback" << endl;
    });
    TcpServer.SetConnectionCallback([](const TcpConnectionSPtr&){
        cout << "TcpServer connection callback" << endl;
    });
    TcpServer.SetCloseCallback([](const TcpConnectionSPtr&){
        cout << "TcpServer close callback" << endl;
    });
    TcpServer.SetErrorCallback([](const TcpConnectionSPtr&){
        cout << "TcpServer error callback" << endl;
    });
    TcpServer.Start();
    cout << "--------------base_loop id = " << base_loop.ThreadId() << endl;

    base_loop.Looping();
    return 0;
}
