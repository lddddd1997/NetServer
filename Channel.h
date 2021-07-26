/**
* @file     Channel.h
* @brief    事件的封装Channel
* @author   lddddd (https://github.com/lddddd1997)
*/
#ifndef CHANNEL_H_
#define CHANNEL_H_

#include <functional>

class Channel
{
public:
    using EventCallback = std::function<void()>;

    Channel();
    ~Channel();

    void SetFd(int fd) // 设置文件描述符
    {
        fd_ = fd;
    }

    int Fd() const // 获取文件描述符
    {
        return fd_;
    }

    void SetEvents(uint32_t events) // 设置触发事件
    {
        events_ = events;
    }

    uint32_t Events() const // 获取设置的触发事件
    {
        return events_;
    }

    void SetRevents(uint32_t revents) // 设置epoll的触发事件
    {
        revents_ = revents;
    }
    uint32_t Revents() const // 获取epoll的触发事件
    {
        return revents_;
    }

    void SetReadHandle(const EventCallback& cb) // 设置读事件回调
    {
        read_callback_ = cb;
    }

    void SetWriteHandle(const EventCallback& cb) // 设置写事件回调
    {
        write_callback_ = cb;
    }

    void SetCloseHandle(const EventCallback& cb) // 设置关闭事件回调
    {
        close_callback_ = cb;
    }

    void SetErrorHandle(const EventCallback& cb) // 设置错误事件回调
    {
        error_callback_ = cb;
    }

    void HandleEvents(); // 事件的处理

private:
    int fd_;
    uint32_t events_;
    uint32_t revents_;

    EventCallback read_callback_;
    EventCallback write_callback_;
    EventCallback close_callback_;
    EventCallback error_callback_;
};

#endif
