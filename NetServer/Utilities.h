/**
* @file     Utilities.h
* @brief    Utilities
* @author   lddddd (https://github.com/lddddd1997)
*/
#ifndef UTILITIES_H_
#define UTILITIES_H_

#include <string>

class Utilities
{
public:
    Utilities();
    ~Utilities();
    
    static const int BUFESIZE = 4096; // 单次最大读写长度
    static void SetNonBlock(int fd); // 设置文件描述符为非阻塞
    static ssize_t Readn(int fd, std::string& buffer_in); // 循环读取，直到error == EAGAIN
    static ssize_t Writen(int fd, std::string& buffer_out); // 循环写，直到error == EAGAIN
};

#endif
