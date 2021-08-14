/**
* @file     Socket.cpp
* @brief    socket封装
* @author   lddddd (https://github.com/lddddd1997)
* @par      bug fixed:
*           2021.08.13，使用一个空闲描述符来解决文件描述符超出限制的问题
*/
#include <netinet/tcp.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <assert.h>
#include <iostream>
#include "Utilities.h"
#include "Socket.h"
#include "Logger.h"

Socket::Socket()
{
    sockfd_ = socket(AF_INET, SOCK_STREAM, 0);
    idlefd_ = open("/dev/null", O_RDONLY | O_CLOEXEC);
    if(sockfd_ == -1 || idlefd_ == -1)
    {
        perror("Socket::Socket");
        exit(EXIT_FAILURE);
    }
    // std::cout << "socket create..." << std::endl;
}

Socket::~Socket()
{
    close(sockfd_);
    close(idlefd_);
    std::cout << "socket close..." << std::endl;
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
void Socket::SetNonblock()
{
    Utilities::SetNonBlock(sockfd_); // 设置非阻塞IO
}

void Socket::BindAddress(uint16_t port) // 绑定socket地址
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

void Socket::BindAddress(const struct sockaddr_in& addr)
{
    if(bind(sockfd_, reinterpret_cast<const struct sockaddr*>(&addr), sizeof(addr)) == -1)
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

int Socket::Accept(struct sockaddr_in& client_addr)
{
    socklen_t addr_len = sizeof(client_addr);
    int connfd = accept(sockfd_, reinterpret_cast<struct sockaddr*>(&client_addr), &addr_len);
    if(connfd == -1)
    {
        // LOG_ERROR << "Resource temporarily unavailable";
        switch(errno)
        {
            case EAGAIN:
                return 0;
            case EMFILE: // 使用一个空闲文件描述符来解决文件描述符超出限制，导致无法服务端继续监听处理新的连接的问题
            {
                close(idlefd_);
                idlefd_ = accept(sockfd_, nullptr, nullptr);
                assert(idlefd_ >= 0);
                close(idlefd_);
                idlefd_ = open("/dev/null", O_RDONLY | O_CLOEXEC);
                assert(idlefd_ >= 0);
                // LOG_ERROR << "Too many open files";
                // std::cout << "Too many open files" << std::endl;
                return 0;
            }
            case ECONNABORTED:
            case EINTR:
            case EPROTO:
            case EPERM:
            case EBADF:
            case EFAULT:
            case EINVAL:
            case ENFILE:
            case ENOBUFS:
            case ENOMEM:
            case ENOTSOCK:
            case EOPNOTSUPP:
                // unexpected errors
                std::cout << "unexpected error of ::accept " << std::endl;
                break;
            default:
                // std::cout << "unknown error of ::accept " << std::endl;
                break;
        }
    }
    return connfd;
}
