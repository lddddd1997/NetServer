/**
* @file     TcpServer.cpp
* @brief    tcp服务器，管理tcp客户端的连接
* @author   lddddd (https://github.com/lddddd1997)
* @par      bug fixed:
*           2021.08.13，使用一个空闲描述符来解决文件描述符超出限制的问题
*/
#include <unistd.h>
#include <cstring>
#include <iostream>
#include "Utilities.h"
#include "TcpServer.h"
#include "Logger.h"

TcpServer::TcpServer(EventLoop *basic_loop, uint16_t port, int thread_num, int idle_seconds) :
    basic_loop_(basic_loop),
    server_channel_("server"),
    event_loop_thread_pool_(basic_loop, thread_num),
    idle_seconds_(idle_seconds),
    timing_wheel_(idle_seconds == 0 ? 1 : idle_seconds)
{
    bzero(&server_addr_, sizeof(server_addr_));
    server_addr_.sin_family = AF_INET;
    server_addr_.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr_.sin_port = htons(port);
    server_socket_.SetNonblock();
    server_socket_.SetReuseAddr(true);
    server_socket_.BindAddress(server_addr_);
    // server_socket_.BindAddress(port);
    server_socket_.SetListen();

    server_channel_.SetFd(server_socket_.Fd());
    server_channel_.SetEvents(EPOLLIN/* | EPOLLET*/); // 电平触发
    server_channel_.SetReadHandler(std::bind(&TcpServer::NewConnectionHandler, this));
    server_channel_.SetErrorHandler(std::bind(&TcpServer::ConnectionErrorHandler, this));
}

TcpServer::~TcpServer()
{
    // basic_loop_->CommitTaskToLoop(std::bind(&EventLoop::RemoveChannelFromEpoller, basic_loop_, &server_channel_)); // close后epoll自动清除，man文档Q6
    close(server_socket_.Fd());
}

void TcpServer::Start()
{
    event_loop_thread_pool_.Start();
    basic_loop_->CommitTaskToLoop(std::bind(&EventLoop::CommitChannelToEpoller, basic_loop_, &server_channel_));
    if(idle_seconds_ != 0) // 设置0则不使用时间轮
    {
        basic_loop_->RunEvery(1.0, std::bind(&TcpServer::CheckIdleConnection, this));
        timing_wheel_.Start();
    }
}

void TcpServer::NewConnectionHandler() // server_channel的EPOLLIN事件触发
{
    struct sockaddr_in client_addr;
    int client_fd = 0;
    if((client_fd = server_socket_.Accept(client_addr)) > 0) // Fix bug:设置为水平触发,处理文件描述符打开数量超过系统限制的情况
    {
        if(connections_map_.size() >= MAX_CONNECTION)
        {
            LOG_WARN << "Exceed the maximum number of simultaneous connections";
            close(client_fd);
            return ;
        }
        Utilities::SetNonBlock(client_fd); // 设置非阻塞IO
        EventLoop *io_loop = event_loop_thread_pool_.GetNextLoop(); // 给新连接分配IO线程
        TcpConnectionSPtr new_connection = std::make_shared<TcpConnection>(io_loop, client_fd, client_addr, server_addr_);
        new_connection->SetMessageCallback(message_callback_);
        if(idle_seconds_ != 0)
        {
            new_connection->SetUpdateCallback(std::bind(&TcpServer::UpdateTimingWheel, this, std::placeholders::_1));
            new_connection->SetCommitCallback(std::bind(&TcpServer::CommitNewConnectionToTimingWheel, this, std::placeholders::_1));
        }
        new_connection->SetWriteCompleteCallback(write_complete_callback_);
        new_connection->SetCloseCallback(close_callback_);
        new_connection->SetErrorCallback(error_callback_);
        new_connection->SetConnectionCleanup(std::bind(&TcpServer::RemoveConnectionFromMap, this, std::placeholders::_1));

        // {
        //     std::lock_guard<std::mutex> lock(mutex_);
        //     connections_map_[client_fd] = new_connection;
        // }
        connections_map_[client_fd] = new_connection; // 加入hash map连接表中

        // LOG_INFO << "New    connection " << inet_ntoa(client_addr.sin_addr) << ":" << ntohs(client_addr.sin_port)
        //         << " from " << inet_ntoa(server_addr_.sin_addr) << ":" << ntohs(server_addr_.sin_port)
        //         << ", client count = " << connections_map_.size() << " client fd = " << client_fd;
        // new_connection->ConnectEstablished(); // 初始化
        io_loop->CommitTaskToLoop(std::bind(&TcpConnection::ConnectEstablished, new_connection));
        connection_callback_(new_connection);
    }
}

void TcpServer::ConnectionErrorHandler()
{
    LOG_ERROR << "Connection error";
    close(server_socket_.Fd());
}

void TcpServer::RemoveConnectionFromMap(const TcpConnectionSPtr& connection)
{
    basic_loop_->CommitTaskToLoop(std::bind(&TcpServer::RemoveConnectionInLoop, this, connection)); // 投递到basic_loop_，解决线程安全问题
    // LOG_INFO << "Remove connection " << inet_ntoa(connection->LocalAddress().sin_addr)
    //         << ":" << ntohs(connection->LocalAddress().sin_port)
    //         << " from " << inet_ntoa(connection->PeerAddress().sin_addr)
    //         << ":" << ntohs(connection->PeerAddress().sin_port); // 放在TcpConection的析构中，析构后才真正回收
}

void TcpServer::RemoveConnectionInLoop(const TcpConnectionSPtr& connection)
{
    // std::lock_guard<std::mutex> lock(mutex_); // 投递到basic_loop_，不用上锁
    connections_map_.erase(connection->Fd());
}

void TcpServer::CommitNewConnectionToTimingWheel(const TcpConnectionSPtr& connection)
{
    timing_wheel_.CommitNewConnection(connection);
}

void TcpServer::UpdateTimingWheel(const TcpConnectionSPtr& connection)
{
    timing_wheel_.Update(connection);
}

void TcpServer::CheckIdleConnection()
{
    timing_wheel_.TimeLapse();
}