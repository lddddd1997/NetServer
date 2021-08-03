/**
* @file     TcpServer.cpp
* @brief    tcp服务器，管理tcp客户端的连接
* @author   lddddd (https://github.com/lddddd1997)
*/
#include <unistd.h>
#include <cstring>
#include <iostream>
#include "Utilities.h"
#include "TcpServer.h"

TcpServer::TcpServer(EventLoop *basic_loop, int port, int thread_num) :
    // conncount_(0),
    basic_loop_(basic_loop),
    server_channel_("server"),
    event_loop_thread_pool_(basic_loop, thread_num)
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
    // basic_loop_->CommitTaskToLoop(std::bind(&EventLoop::RemoveChannelFromEpoller, basic_loop_, &server_channel_)); // close后epoll自动清除，man文档Q6
    close(server_socket_.Fd());
}

void TcpServer::Start()
{
    event_loop_thread_pool_.Start();
    basic_loop_->CommitTaskToLoop(std::bind(&EventLoop::CommitChannelToEpoller, basic_loop_, &server_channel_));
}

void TcpServer::NewConnectionHandler() // server_channel的EPOLLIN事件触发
{
    struct sockaddr_in client_addr;
    int client_fd = 0;
    while((client_fd = server_socket_.Accept(client_addr)) > 0) // 非阻塞处理
    {
        if(connections_map_.size() >= MAXCONNECTION)
        {
            close(client_fd);
            continue;
        }
        Utilities::SetNonBlock(client_fd); // 设置非阻塞IO
        EventLoop *io_loop = event_loop_thread_pool_.GetNextLoop(); // 给新连接分配IO线程
        TcpConnectionSPtr new_connection = std::make_shared<TcpConnection>(io_loop, client_fd, client_addr, server_addr_);
        new_connection->SetMessageCallback(message_callback_);
        new_connection->SetWriteCompleteCallback(write_complete_callback_);
        new_connection->SetCloseCallback(close_callback_);
        new_connection->SetErrorCallback(error_callback_);
        new_connection->SetConnectionCleanup(std::bind(&TcpServer::RemoveConnectionFromMap, this, std::placeholders::_1));

        // {
        //     std::lock_guard<std::mutex> lock(mutex_);
        //     connections_map_[client_fd] = new_connection;
        // }
        connections_map_[client_fd] = new_connection; // 加入hash map连接表中
        new_connection->ConnectEstablished(); // 初始化
        connection_callback_(new_connection);
        // std::cout << "New client connection, address = " << inet_ntoa(client_addr.sin_addr) << 
        // ":" << ntohs(client_addr.sin_port) << " client count = " << connections_map_.size() << std::endl;
        // std::cout << "Server, address = " << inet_ntoa(server_addr_.sin_addr) << 
        // ":" << ntohs(server_addr_.sin_port) << " client_fd = " << client_fd << std::endl;
    }
}

void TcpServer::ConnectionErrorHandler()
{
    std::cout << "connection error" << std::endl;
    close(server_socket_.Fd());
}

void TcpServer::RemoveConnectionFromMap(const TcpConnectionSPtr& connection)
{
    basic_loop_->CommitTaskToLoop(std::bind(&TcpServer::RemoveConnectionInLoop, this, connection)); // 投递到basic_loop_，解决线程安全问题
}

void TcpServer::RemoveConnectionInLoop(const TcpConnectionSPtr& connection)
{
    // std::lock_guard<std::mutex> lock(mutex_); // 投递到basic_loop_，不用上锁
    connections_map_.erase(connection->Fd());
}

