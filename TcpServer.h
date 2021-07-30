/**
* @file     TcpServer.h
* @brief    tcp服务器，管理tcp客户端的连接
* @author   lddddd (https://github.com/lddddd1997)
*/

#ifndef TCP_SERVER_H_
#define TCP_SERVER_H_

#include <memory>
#include <string>
#include <functional>
#include <EventLoopThreadPool.h>
#include <TcpConnection.h>
#include <Socket.h>

class TcpServer
{
public:
    using TcpConnectionSPtr = std::shared_ptr<TcpConnection>;
    using ConnectionHashMap = std::unordered_map<int, TcpConnectionSPtr>;
    using Callback = std::function<void(const TcpConnectionSPtr&)>;
    using MessageCallback = std::function<void(const TcpConnectionSPtr&, std::string&)>;

    TcpServer(EventLoop *base_loop, int port, int threadnum);
    ~TcpServer();

    void Start();
    void SetMessageCallback(const MessageCallback& cb)
    {
        message_callback_ = cb;
    }

    void SetWriteCompleteCallback(const Callback &cb)
    { 
        write_complete_callback_ = cb; 
    }
    void SetConnectionCallback(const Callback& cb)
    {
        connection_callback_ = cb;
    }

    void SetCloseCallback(const Callback& cb)
    {
        close_callback_ = cb;
    }

    void SetErrorCallback(const Callback& cb)
    {
        error_callback_ = cb;
    }

private:
    // int connection_count_;
    EventLoop *base_loop_;
    Socket server_socket_;
    Channel server_channel_;
    struct sockaddr_in server_addr_;
    ConnectionHashMap connections_map_;
    EventLoopThreadPool event_loop_thread_pool_;
    // std::mutex mutex_; // 解决connections_map_的线程安全

    MessageCallback message_callback_;
    Callback write_complete_callback_;
    Callback connection_callback_;
    Callback close_callback_;
    Callback error_callback_;

    static const int MAXCONNECTION = 30000;

    void NewConnectionHandler();
    void ConnectionErrorHandler();
    void RemoveConnectionFromMap(const TcpConnectionSPtr& connection);
    void RemoveConnectionInLoop(const TcpConnectionSPtr& connection);
};

#endif
