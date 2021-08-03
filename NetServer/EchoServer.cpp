/**
* @file     EchoServer.cpp
* @brief    echo服务器
* @author   lddddd (https://github.com/lddddd1997)
*/
#include <EchoServer.h>
#include <iostream>

EchoServer::EchoServer(EventLoop *basic_loop, int port, int thread_num) :
    tcp_server_(basic_loop, port, thread_num)
{
    tcp_server_.SetMessageCallback(std::bind(&EchoServer::MessageCallback, this, std::placeholders::_1, std::placeholders::_2));
    tcp_server_.SetWriteCompleteCallback(std::bind(&EchoServer::WriteCompleteCallback, this, std::placeholders::_1));
    tcp_server_.SetConnectionCallback(std::bind(&EchoServer::ConnectionCallback, this, std::placeholders::_1));
    tcp_server_.SetCloseCallback(std::bind(&EchoServer::CloseCallback, this, std::placeholders::_1));
    tcp_server_.SetErrorCallback(std::bind(&EchoServer::ErrorCallback, this, std::placeholders::_1));
}

EchoServer::~EchoServer()
{

}

void EchoServer::Start()
{
    std::cout << "Echo server start..." << std::endl;
    tcp_server_.Start();
}

void EchoServer::MessageCallback(const TcpConnectionSPtr& connection, std::string& message)
{
    std::string msg;
    msg.swap(message);
    std::cout << "receive " << msg.size() - 1 << " byte massage from client address = " << inet_ntoa(connection->LocalAddress().sin_addr) << 
    ":" << ntohs(connection->LocalAddress().sin_port) << std::endl << ">> " << msg << std::endl;
    connection->Send(msg);
    connection->Shutdown();
}

void EchoServer::WriteCompleteCallback(const TcpConnectionSPtr& connection)
{
    // std::cout << "-----------------TcpServer write complete callback" << std::endl;
}

void EchoServer::ConnectionCallback(const TcpConnectionSPtr& connection)
{
    std::cout << "New client connection, address = " << inet_ntoa(connection->LocalAddress().sin_addr) << 
    ":" << ntohs(connection->LocalAddress().sin_port) << ", client count = " << ClientsCount() << std::endl;
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
