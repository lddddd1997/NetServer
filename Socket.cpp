/**
* @file     Socket.cpp
* @brief    socket
* @author   lddddd (https://github.com/lddddd1997)
*/
#include <Socket.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

Socket::Socket()
{
    sockfd_ = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd_ == -1)
    {
        perror("Socket::Socket");
        exit(EXIT_FAILURE);
    }
    std::cout << "socket create..." << std::endl;
}

Socket::~Socket()
{
    close(sockfd_);
    std::cout << "socket close..." << std::endl;
}

int Socket::Fd() const
{
    return sockfd_;
}

void Socket::SetReuseAddr(bool on)
{
    int opt = on ? 1 : 0;
    if(setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &opt, static_cast<socklen_t>(sizeof(opt))) == -1)
    {
        perror("Socket::SetReuseAddr");
        exit(EXIT_FAILURE);
    }
}

void Socket::SetReusePort(bool on)
{
    int opt = on ? 1 : 0;
    if(setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, &opt, static_cast<socklen_t>(sizeof(opt))) == -1)
    {
        perror("Socket::SetReusePort");
        exit(EXIT_FAILURE);
    }
}

void Socket::SetKeepAlive(bool on)
{
    int opt = on ? 1 : 0;
    if(setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &opt, static_cast<socklen_t>(sizeof(opt))) == -1)
    {
        perror("Socket::SetKeepAlive");
        exit(EXIT_FAILURE);
    }
}

void Socket::SetTcpNoDelay(bool on) // 关闭Nagle算法
{
    int opt = on ? 1 : 0;
    if(setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &opt, static_cast<socklen_t>(sizeof(opt))) == -1)
    {
        perror("Socket::SetTcpNoDelay");
        exit(EXIT_FAILURE);
    }
}

void Socket::BindAddress(int port)
{
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);
    if(bind(sockfd_, reinterpret_cast<struct sockaddr*>(&server_addr), sizeof(server_addr)) == -1)
    {
        close(sockfd_);
        perror("Socket::BindAddress");
        exit(EXIT_FAILURE);
    }
}

void Socket::SetListen()
{
    if(listen(sockfd_, SOMAXCONN) == -1) // 设置最大同时等待连接数，而非同时保持连接数量
    {
        perror("Socket::SetListen");
        exit(EXIT_FAILURE);
    }
}

int Socket::Accept(struct sockaddr_in &client_addr)
{
    socklen_t addr_len = sizeof(client_addr);
    int connfd = accept4(sockfd_, reinterpret_cast<struct sockaddr*>(&client_addr),
                         &addr_len, SOCK_NONBLOCK | SOCK_CLOEXEC); // accept4相较于accpet多了设置flags参数
    if(connfd == -1)
    {
        perror("Socket::Accept");
        switch(errno)
        {
            case EAGAIN:
                return 0;
            case ECONNABORTED:
            case EINTR:
            case EPROTO:
            case EPERM:
            case EMFILE:
                // expected errors
                break;
            case EBADF:
            case EFAULT:
            case EINVAL:
            case ENFILE:
            case ENOBUFS:
            case ENOMEM:
            case ENOTSOCK:
            case EOPNOTSUPP:
                // unexpected errors
                // std::cout << "unexpected error of ::accept " << std::endl;
                break;
            default:
                // std::cout << "unknown error of ::accept " << std::endl;
                break;
        }
    }
    return connfd;
}
