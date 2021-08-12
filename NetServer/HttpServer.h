/**
* @file     HttpServer.h
* @brief    http服务器
* @author   lddddd (https://github.com/lddddd1997)
*/
#ifndef HTTP_SERVER_H_
#define HTTP_SERVER_H_

#include "EventLoop.h"
#include "TcpServer.h"
#include "ThreadPool.h"

class HttpServer
{
public:
    using TcpConnectionSPtr = std::shared_ptr<TcpConnection>;

    HttpServer(EventLoop *basic_loop, uint16_t port, int io_thread_num, int worker_thread_num, int idle_seconds);
    ~HttpServer();

    void Start();

private:
    TcpServer tcp_server_;
    ThreadPool worker_thread_pool_;

    void OnMessage(const TcpConnectionSPtr& connection, std::string& message);
    void OnWriteComplete(const TcpConnectionSPtr& connection);
    void OnNewConnection(const TcpConnectionSPtr& connection);
    void OnConnectionClose(const TcpConnectionSPtr& connection);
    void OnError(const TcpConnectionSPtr& connection);
    void OnRequestProcessing(const TcpConnectionSPtr& connection, std::string& message);
};

#endif
