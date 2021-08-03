/**
* @file     TcpConnection.h
* @brief    tcp客户端连接，上层使用shared_ptr进行生命周期管理
* @author   lddddd (https://github.com/lddddd1997)
*/
#ifndef TCP_CONNECTION_H_
#define TCP_CONNECTION_H_

#include <string>
#include <memory>
#include <arpa/inet.h>
#include <boost/any.hpp>
#include "Channel.h"
#include "EventLoop.h"

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
    
    int Fd() const // 获取客户端的文件描述符
    {
        // return fd_;
        return channel_->Fd();
    }
    bool Connected()
    {
        return !disconnected_;
    }
    EventLoop* Loop() const // 获取客户端所处的loop线程
    {
        return loop_;
    }
    void SetContext(const boost::any& context)
    {
        context_ = context;
    }
    const boost::any& Context() const
    {
        return context_;
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

    void ConnectEstablished(); // 连接建立初始化，将客户端channel加入epoller
    void Send(const std::string& str); // 应用层调用的数据发送接口
    void Shutdown(); // 应用层调用的主动关闭连接接口

private:
    std::atomic<bool> disconnected_;
    // int fd_;
    EventLoop *loop_;
    ChannelUPtr channel_;
    boost::any context_;
    const struct sockaddr_in local_addr_; // 客户端的socket地址
    const struct sockaddr_in peer_addr_; // 服务端的socket地址
    std::string buffer_in_; // 接收缓冲区
    std::string buffer_out_;  // 发送缓冲区
    
    MessageCallback message_callback_; // 应用层消息回调
    Callback write_complete_callback_; // 应用层写完回调
    Callback close_callback_; // 应用层关闭连接回调
    Callback error_callback_; // 应用层错误回调
    Callback connection_cleanup_; // TcpServer层清理连接回调

    void SendInLoop(); // 将Send投递到连接所处的loop线程
    void ShutdownInLoop(); // 将Shutdown投递到连接所处的loop线程
    void ReadHandler(); // channel各事件发生的处理
    void WriteHandler();
    void CloseHandler();
    void ErrorHandler();
};

#endif
