/**
* @file     Utilities.cpp
* @brief    Utilities
* @author   lddddd (https://github.com/lddddd1997)
*/
#include <unistd.h>
#include <fcntl.h>
#include "Utilities.h"
#include "Logger.h"

Utilities::Utilities()
{

}

Utilities::~Utilities()
{

}

void Utilities::SetNonBlock(int fd)
{
    int opt = fcntl(fd, F_GETFL);
    if(opt < 0)
    {
        perror("Utilities::SetNonBlock");
        exit(EXIT_FAILURE);
    }
    if(fcntl(fd, F_SETFL, opt | O_NONBLOCK) < 0)
    {
        perror("Utilities::SetNonBlock");
        exit(EXIT_FAILURE);
    }
}

ssize_t Utilities::Readn(int fd, std::string& buffer_in)
{
    ssize_t nread = 0;
    ssize_t read_sum = 0;
    char buff[BUFER_SIZE];
    while(true)
    {
        if((nread = read(fd, buff, BUFER_SIZE)) < 0)
        {
            if(errno == EAGAIN) // 系统缓冲区没有数据，非阻塞返回
            {
                break;
            }
            else if(errno == EINTR)
            {
                continue;
            }
            else
            {
                // perror("Utilities::Readn");
                LOG_ERROR << "Utilities::Readn";
                return -1;
            }
        }
        else if(nread == 0) // 客户端关闭socket，FIN
        {
            return 0;
        }
        read_sum += nread;
        buffer_in += std::string(buff, buff + nread);
    }
    return read_sum;
}

ssize_t Utilities::Writen(int fd, std::string& buffer_out)
{
    ssize_t nleft = buffer_out.size();
    ssize_t nwritten = 0;
    ssize_t written_sum = 0;
    const char *ptr = buffer_out.c_str();
    while(nleft > 0)
    {
        if((nwritten = write(fd, ptr, nleft)) <= 0)
        {
            if(nwritten < 0)
            {
                if(errno == EAGAIN) // 系统缓冲区满，非阻塞返回
                {
                    break;
                }
                else if(errno == EINTR)
                {
                    continue;
                }
                else
                {
                    // perror("Utilities::Writen");
                    LOG_ERROR << "Utilities::Writen";
                    return -1;
                }
            }
            else
            {
                return 0;
            }
        }
        written_sum += nwritten;
        ptr += nwritten;
        nleft -= nwritten;
    }
    buffer_out = buffer_out.substr(written_sum);
    return written_sum;
}

std::string Utilities::Hostname()
    {
        // HOST_NAME_MAX 64
        // _POSIX_HOST_NAME_MAX 255
        char buf[256];
        if (gethostname(buf, sizeof buf) == 0)
        {
            buf[sizeof(buf)-1] = '\0';
            return buf;
        }
        else
        {
            return "unknownhost";
        }
    }

pid_t Utilities::Pid()
{
    return getpid();
}
