/**
* @file     EchoServer.h
* @brief    echo服务器
* @author   lddddd (https://github.com/lddddd1997)
*/
#ifndef ECHO_SERVER_H_
#define ECHO_SERVER_H_

#include "TcpServer.h"
#include "TimingWheel.h"

class EchoServer
{
public:
    using TcpConnectionSPtr = std::shared_ptr<TcpConnection>;

    EchoServer(EventLoop *basic_loop, int port, int thread_num, int idle_seconds);
    ~EchoServer();
    
    void Start();
    int ClientsCount() const
    {
        return tcp_server_.ConnectionsCount();
    }

private:
    TcpServer tcp_server_;
    TimingWheel timing_wheel_;
    void OnTimer();
    void MessageCallback(const TcpConnectionSPtr& connection, std::string& message);
    void WriteCompleteCallback(const TcpConnectionSPtr& connection);
    void ConnectionCallback(const TcpConnectionSPtr& connection);
    void CloseCallback(const TcpConnectionSPtr& connection);
    void ErrorCallback(const TcpConnectionSPtr& connection);
};

#endif
