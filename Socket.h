/**
* @file     Socket.h
* @brief    socket
* @author   lddddd (https://github.com/lddddd1997)
*/

#include <arpa/inet.h>

#ifndef SOCKET_H_
#define SOCKET_H_

class Socket
{
public:
    Socket();
    Socket(const Socket&) = delete;
    ~Socket();
    Socket& operator=(const Socket&) = delete;

    int Fd() const
    {
        return sockfd_;
    }
    
    void SetReuseAddr(bool on);
    void SetReusePort(bool on);
    void SetKeepAlive(bool on);
    void SetTcpNoDelay(bool on);
    void BindAddress(int port);
    void SetListen();
    int Accept(struct sockaddr_in &client_addr);
private:
    int sockfd_;
};

#endif
