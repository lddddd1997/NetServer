/**
* @file     Channel.h
* @brief    事件的封装Channel
* @author   lddddd (https://github.com/lddddd1997)
*/
#ifndef CHANNEL_H_
#define CHANNEL_H_

#include <functional>
#include <string>
#include <memory>

class Channel
{
public:
    using EventHandler = std::function<void()>;

    Channel(const std::string& name);
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
    void SetReadHandler(const EventHandler& cb) // 设置读事件回调
    {
        read_handler_ = cb;
    }
    void SetWriteHandler(const EventHandler& cb) // 设置写事件回调
    {
        write_handler_ = cb;
    }
    void SetCloseHandler(const EventHandler& cb) // 设置关闭事件回调
    {
        close_handler_ = cb;
    }
    void SetErrorHandler(const EventHandler& cb) // 设置错误事件回调
    {
        error_handler_ = cb;
    }
    void Tie(const std::shared_ptr<void>& obj)
    {
        tie_ = obj;
        tied_ = true;
    }

    void EventsHandlingWithGuard(); // 事件的处理

private:
    int fd_;
    uint32_t events_;
    uint32_t revents_;
    std::string name_;

    bool tied_;
    std::weak_ptr<void> tie_;

    EventHandler read_handler_; // 事件处理回调
    EventHandler write_handler_;
    EventHandler close_handler_;
    EventHandler error_handler_;
};

#endif
