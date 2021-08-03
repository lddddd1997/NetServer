#ifndef UTILITIES_H_
#define UTILITIES_H_

#include <string>

class Utilities
{
public:
    Utilities();
    ~Utilities();
    
    static const int BUFESIZE = 4096;
    static void SetNonBlock(int fd);
    static ssize_t Readn(int fd, std::string& buffer_in);
    static ssize_t Writen(int fd, std::string& buffer_out);
};

#endif
