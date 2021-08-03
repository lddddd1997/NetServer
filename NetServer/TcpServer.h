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

    TcpServer(EventLoop *basic_loop, int port, int threa_dnum);
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

    int ConnectionsCount() const // 可能存在线程安全？？？
    {
        return connections_map_.size();
    }
private:
    // int connection_count_;
    EventLoop *basic_loop_; // 服务端使用主线程监听TCP连接，分发给IO线程
    Socket server_socket_; // 服务端socket
    Channel server_channel_; // 服务端channel
    struct sockaddr_in server_addr_; // 服务端地址
    ConnectionHashMap connections_map_; // 客户端连接表
    EventLoopThreadPool event_loop_thread_pool_; // IO线程池，处理客户端的IO请求
    // std::mutex mutex_; // 解决connections_map_的线程安全

    MessageCallback message_callback_;  // 应用层消息接收回调
    Callback write_complete_callback_; // 应用层写完回调
    Callback connection_callback_; // 应用层新连接回调
    Callback close_callback_; // 应用层关闭连接回调
    Callback error_callback_; // 应用层错误回调

    static const int MAXCONNECTION = 30000; // 限制最大连接数量

    void NewConnectionHandler(); // socket可读（有新连接）处理
    void ConnectionErrorHandler();
    void RemoveConnectionFromMap(const TcpConnectionSPtr& connection); // 客户端文件描述符断开处理，从连接表中删除该连接
    void RemoveConnectionInLoop(const TcpConnectionSPtr& connection); // 将删除任务投递到所在线程
};

#endif
