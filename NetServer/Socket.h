/**
* @file     Socket.h
* @brief    socket封装
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

    int Fd() const // 获取socket文件描述符
    {
        return sockfd_;
    }
    
    void SetReuseAddr(bool on); // 设置地址复用
    void SetReusePort(bool on); // 设置端口复用
    void SetKeepAlive(bool on); // 设置TCP层的心跳检测，默认发送心跳检测数据包间隔为7200s
    void SetTcpNoDelay(bool on); // 设置是否开启nagle算法，关闭后提高传输效率
    void SetNonblock(); // 设置非阻塞IO
    void BindAddress(int port); // 绑定socket地址
    void BindAddress(const struct sockaddr_in& addr); // 绑定socket地址
    void SetListen(); // 设置最大同时等待连接数
    int Accept(struct sockaddr_in &client_addr); // accept封装
private:
    int sockfd_; // socket文件描述符
};

#endif
