/**
* @file     TcpConnection.h
* @brief    tcp connection
* @author   lddddd (https://github.com/lddddd1997)
*/
#ifndef TCP_CONNECTION_H_
#define TCP_CONNECTION_H_

#include <string>
#include <memory>
#include <Channel.h>
#include <EventLoop.h>
#include <arpa/inet.h>

class TcpConnection : public std::enable_shared_from_this<TcpConnection>
{
public:
    using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
    using Callback = std::function<void (const TcpConnectionPtr&)>;

    TcpConnection(EventLoop *loop, int fd_,
                  const struct sockaddr_in& local_addr, const struct sockaddr_in& peer_addr);
    ~TcpConnection();
    
    int Fd() const
    {
        return fd_;
    }

    EventLoop *GetLoop() const
    {
        return loop_;
    }

    void CommitChannelToLoop();

private:
    int fd_;
    EventLoop *loop_;
    std::unique_ptr<Channel> channel_;
    // const struct sockaddr_in local_addr_;
    // const struct sockaddr_in Peer_addr_;
    struct sockaddr_in local_addr_;
    struct sockaddr_in peer_addr_;
    std::string buffer_in_;
    std::string buffer_out_; 
    
    void HandleRead();
    void HandleWrite();
    void HandleClose();
    void HandleError();
};

#endif
