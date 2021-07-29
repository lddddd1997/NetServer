/**
* @file     TcpServer.cpp
* @brief    tcp服务器，管理tcp客户端的连接
* @author   lddddd (https://github.com/lddddd1997)
*/
#include <TcpServer.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

TcpServer::TcpServer(EventLoop *base_loop, int port, int threadnum) :
    // conncount_(0),
    base_loop_(base_loop),
    event_loop_thread_pool_(base_loop, threadnum)
{
    bzero(&server_addr_, sizeof(server_addr_));
    server_addr_.sin_family = AF_INET;
    server_addr_.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr_.sin_port = htons(port);
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
        ":" << ntohs(client_addr.sin_port) << std::endl;
        if(connections_map_.size() >= MAXCONNECTION)
        {
            close(client_fd);
            continue;
        }
        EventLoop *loop = event_loop_thread_pool_.GetNextLoop();
        TcpConnectionSPtr new_connection = std::make_shared<TcpConnection>(loop, client_fd, server_addr_, client_addr);
        new_connection->SetMessaeCallback(message_callback_);
        new_connection->SetWriteCompleteCallback(write_complete_callback_);
        new_connection->SetCloseCallback(close_callback_);
        new_connection->SetErrorCallback(error_callback_);
        new_connection->SetConnectionCleanup(std::bind(&TcpServer::RemoveConnection, this, std::placeholders::_1));
        
        {
            std::lock_guard<std::mutex> lock(mutex_);
            connections_map_[client_fd] = new_connection; // 存在线程安全吗？？？
        }
        connection_callback_(new_connection);
        new_connection->ConnectEstablished();
    }
}

void TcpServer::ConnectionErrorHandler()
{
    std::cout << "connection error" << std::endl;
    close(server_socket_.Fd());
}

void TcpServer::RemoveConnection(const TcpConnectionSPtr& connection)
{
    std::lock_guard<std::mutex> lock(mutex_);
    connections_map_.erase(connection->Fd());
}
