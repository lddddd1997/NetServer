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

    void SetFd(int fd);
    int Fd() const;
    void SetEvents(uint32_t events);
    uint32_t Events() const;

    void SetReadHandle(const EventCallback& cb);
    void SetWriteHandle(const EventCallback& cb);
    void SetErrorHandle(const EventCallback& cb);
    void SetCloseHandle(const EventCallback& cb);

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
