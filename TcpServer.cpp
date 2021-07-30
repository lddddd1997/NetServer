/**
* @file     TcpServer.cpp
* @brief    tcp服务器，管理tcp客户端的连接
* @author   lddddd (https://github.com/lddddd1997)
*/
#include <TcpServer.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <Utilities.h>

TcpServer::TcpServer(EventLoop *base_loop, int port, int threadnum) :
    // conncount_(0),
    base_loop_(base_loop),
    server_channel_("server"),
    event_loop_thread_pool_(base_loop, threadnum)
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
    server_channel_.SetEvents(EPOLLIN | EPOLLET);
    server_channel_.SetReadHandler(std::bind(&TcpServer::NewConnectionHandler, this));
    server_channel_.SetErrorHandler(std::bind(&TcpServer::ConnectionErrorHandler, this));
}

TcpServer::~TcpServer()
{
    // base_loop_->CommitTaskToLoop(std::bind(&EventLoop::RemoveChannelFromEpoller, base_loop_, &server_channel_)); // close后epoll自动清除，man文档Q6
    close(server_socket_.Fd());
}

void TcpServer::Start()
{
    event_loop_thread_pool_.Start();

    // base_loop_->CommitChannelToEpoller(&server_channel_); // base_loop_线程
    base_loop_->CommitTaskToLoop(std::bind(&EventLoop::CommitChannelToEpoller, base_loop_, &server_channel_));
}

void TcpServer::NewConnectionHandler() // server_channel的EPOLLIN事件触发
{
    struct sockaddr_in client_addr;
    int client_fd = 0;
    while((client_fd = server_socket_.Accept(client_addr)) > 0) // 非阻塞处理，Accept封装accept4函数
    {
        std::cout << "New client connection, address = " << inet_ntoa(client_addr.sin_addr) << 
        ":" << ntohs(client_addr.sin_port) << " client_fd = " << client_fd << std::endl;
        // std::cout << "Server, address = " << inet_ntoa(server_addr_.sin_addr) << 
        // ":" << ntohs(server_addr_.sin_port) << " client_fd = " << client_fd << std::endl;
        if(connections_map_.size() >= MAXCONNECTION)
        {
            close(client_fd);
            continue;
        }
        Utilities::SetNonBlock(client_fd); // 设置非阻塞IO
        EventLoop *io_loop = event_loop_thread_pool_.GetNextLoop();
        TcpConnectionSPtr new_connection = std::make_shared<TcpConnection>(io_loop, client_fd, server_addr_, client_addr);
        new_connection->SetMessaeCallback(message_callback_);
        new_connection->SetWriteCompleteCallback(write_complete_callback_);
        new_connection->SetCloseCallback(close_callback_);
        new_connection->SetErrorCallback(error_callback_);
        new_connection->SetConnectionCleanup(std::bind(&TcpServer::RemoveConnectionFromMap, this, std::placeholders::_1));

        // {
        //     std::lock_guard<std::mutex> lock(mutex_);
        //     connections_map_[client_fd] = new_connection;
        // }
        connections_map_[client_fd] = new_connection;
        connection_callback_(new_connection);
        new_connection->ConnectEstablished();
        std::cout << "client count = " << connections_map_.size() << std::endl;
    }
}

void TcpServer::ConnectionErrorHandler()
{
    std::cout << "connection error" << std::endl;
    close(server_socket_.Fd());
}

void TcpServer::RemoveConnectionFromMap(const TcpConnectionSPtr& connection)
{
    base_loop_->CommitTaskToLoop(std::bind(&TcpServer::RemoveConnectionInLoop, this, connection)); // 投递到base_loop_，解决线程安全问题
}

void TcpServer::RemoveConnectionInLoop(const TcpConnectionSPtr& connection)
{
    // std::lock_guard<std::mutex> lock(mutex_); // 投递到base_loop_，不用上锁
    connections_map_.erase(connection->Fd());
    std::cout << "RemoveConnectionInLoop " << connection.use_count() << std::endl;
}

