/**
* @file     EchoServer.cpp
* @brief    echo服务器
* @author   lddddd (https://github.com/lddddd1997)
*/
#include <iostream>
#include "EchoServer.h"

EchoServer::EchoServer(EventLoop *basic_loop, int port, int thread_num, int idle_seconds) :
    tcp_server_(basic_loop, port, thread_num),
    timing_wheel_(idle_seconds)
{
    tcp_server_.SetMessageCallback(std::bind(&EchoServer::MessageCallback, this, std::placeholders::_1, std::placeholders::_2));
    tcp_server_.SetWriteCompleteCallback(std::bind(&EchoServer::WriteCompleteCallback, this, std::placeholders::_1));
    tcp_server_.SetConnectionCallback(std::bind(&EchoServer::ConnectionCallback, this, std::placeholders::_1));
    tcp_server_.SetCloseCallback(std::bind(&EchoServer::CloseCallback, this, std::placeholders::_1));
    tcp_server_.SetErrorCallback(std::bind(&EchoServer::ErrorCallback, this, std::placeholders::_1));
    basic_loop->RunEvery(1.0, std::bind(&EchoServer::OnTimer, this));
}

EchoServer::~EchoServer()
{

}

void EchoServer::Start()
{
    std::cout << "Echo server start..." << std::endl;
    timing_wheel_.Start();
    tcp_server_.Start();
}

void EchoServer::OnTimer()
{
    timing_wheel_.TimeLapse();
}

void EchoServer::MessageCallback(const TcpConnectionSPtr& connection, std::string& message)
{
    std::string msg;
    msg.swap(message);
    std::cout << "receive " << msg.size() - 1 << " byte massage from client address = " << inet_ntoa(connection->LocalAddress().sin_addr) << 
    ":" << ntohs(connection->LocalAddress().sin_port) << std::endl << ">> " << msg << std::endl;
    // connection->Send(msg);
    msg.clear();
    msg.resize(32);
    time_t now = time(nullptr);
    struct tm tm_now = *localtime(&now); // 内部锁机制，效率差
    snprintf(&*msg.begin(), msg.size(), "%4d-%02d-%02d %02d:%02d:%02d\n", tm_now.tm_year+1900, tm_now.tm_mon+1, tm_now.tm_mday, tm_now.tm_hour, tm_now.tm_min, tm_now.tm_sec);
    connection->Send(msg);
    timing_wheel_.Update(connection);
}

void EchoServer::WriteCompleteCallback(const TcpConnectionSPtr& connection)
{
    // std::cout << "-----------------TcpServer write complete callback" << std::endl;
}

void EchoServer::ConnectionCallback(const TcpConnectionSPtr& connection)
{
    std::cout << "New client connection, address = " << inet_ntoa(connection->LocalAddress().sin_addr) << 
    ":" << ntohs(connection->LocalAddress().sin_port) << ", client count = " << ClientsCount() << std::endl;
    
    timing_wheel_.CommitNewConnection(connection);
    // std::cout << "-----------------TcpServer connection callback" << std::endl;
}

void EchoServer::CloseCallback(const TcpConnectionSPtr& connection)
{
    std::cout << "client closed, address = " << inet_ntoa(connection->LocalAddress().sin_addr) << 
    ":" << ntohs(connection->LocalAddress().sin_port) << ", client count = " << ClientsCount() << std::endl;
    // std::cout << "-----------------TcpServer close callback" << std::endl;
}

void EchoServer::ErrorCallback(const TcpConnectionSPtr& connection)
{
    std::cout << "client error, address = " << inet_ntoa(connection->LocalAddress().sin_addr) << 
    ":" << ntohs(connection->LocalAddress().sin_port) << ", client count = " << ClientsCount() << std::endl;
    // std::cout << "-----------------TcpServer close callback" << std::endl;
}
