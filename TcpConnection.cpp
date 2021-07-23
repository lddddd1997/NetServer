/**
* @file     TcpConnection.cpp
* @brief    tcp connection
* @author   lddddd (https://github.com/lddddd1997)
*/

#include <TcpConnection.h>

TcpConnection::TcpConnection(EventLoop *loop, int fd_,
                  const struct sockaddr_in& local_addr, const struct sockaddr_in& peer_addr) :
    
    local_addr_(local_addr),
    peer_addr_(peer_addr)
    
{
    
}

TcpConnection::~TcpConnection()
{

}

void TcpConnection::CommitChannelToLoop()
{

}

void TcpConnection::HandleRead()
{

}

void TcpConnection::HandleWrite()
{

}

void TcpConnection::HandleClose()
{

}

void TcpConnection::HandleError()
{

}
