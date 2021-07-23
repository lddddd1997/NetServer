/**
* @file     Channel.h
* @brief    channel
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

    void SetFd(int fd)
    {
        fd_ = fd;
    }

    int Fd() const
    {
        return fd_;
    }

    void SetEvents(uint32_t events)
    {
        events_ = events;
    }

    uint32_t Events() const
    {
        return events_;
    }

    void SetReadHandle(const EventCallback& cb)
    {
        read_callback_ = cb;
    }

    void SetWriteHandle(const EventCallback& cb)
    {
        write_callback_ = cb;
    }

    void SetErrorHandle(const EventCallback& cb)
    {
        close_callback_ = cb;
    }

    void SetCloseHandle(const EventCallback& cb)
    {
        error_callback_ = cb;
    }

    void HandleEvents();

private:
    int fd_;
    uint32_t events_;

    EventCallback read_callback_;
    EventCallback write_callback_;
    EventCallback close_callback_;
    EventCallback error_callback_;
};

#endif
