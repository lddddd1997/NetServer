/**
* @file     TcpConnection.h
* @brief    tcp客户端连接，上层使用shared_ptr进行声明周期管理
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
    using ChannelUPtr = std::unique_ptr<Channel>;
    using TcpConnectionSPtr = std::shared_ptr<TcpConnection>;
    using Callback = std::function<void(const TcpConnectionSPtr&)>;
    using MessageCallback = std::function<void(const TcpConnectionSPtr&, std::string&)>;

    TcpConnection(EventLoop *loop, int fd,
                  const struct sockaddr_in& local_addr, const struct sockaddr_in& peer_addr);
    ~TcpConnection();
    
    int Fd() const
    {
        // return fd_;
        return channel_->Fd();
    }

    EventLoop *Loop() const
    {
        return loop_;
    }
    struct sockaddr_in LocalAddress() const
    {
        return local_addr_;
    }
    struct sockaddr_in PeerAddress() const
    {
        return peer_addr_;
    }
    void SetMessageCallback(const MessageCallback &cb)
    {
        message_callback_ = cb;
    }
    void SetWriteCompleteCallback(const Callback &cb)
    {
        write_complete_callback_ = cb;
    }
    void SetCloseCallback(const Callback &cb)
    {
        close_callback_ = cb;
    }
    void SetErrorCallback(const Callback &cb)
    {
        error_callback_ = cb;
    }
    void SetConnectionCleanup(const Callback &cb)
    {
        connection_cleanup_ = cb;
    }

    void ConnectEstablished();
    void Send(const std::string& str);
    void Shutdown();

private:
    bool disconnected_;
    // int fd_;
    EventLoop *loop_;
    ChannelUPtr channel_;
    const struct sockaddr_in local_addr_;
    const struct sockaddr_in peer_addr_;
    // struct sockaddr_in local_addr_;
    // struct sockaddr_in peer_addr_;
    std::string buffer_in_;
    std::string buffer_out_; 
    
    MessageCallback message_callback_;
    Callback write_complete_callback_;
    Callback close_callback_;
    Callback error_callback_;
    Callback connection_cleanup_;

    void SendInLoop();
    void ShutdownInLoop();
    void ReadHandler();
    void WriteHandler();
    void CloseHandler();
    void ErrorHandler();
};

#endif
