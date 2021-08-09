/**
* @file     LogStream.h
* @brief    日志流封装
* @author   lddddd (https://github.com/lddddd1997)
* TODO:     使用c++20的format替代snprintf的格式化
*/
#ifndef LOG_STREAM_H_
#define LOG_STREAM_H_

#include <cstring>
#include <string>
#include <iostream>

template<int SIZE>
class FixedBuffer
{
public:
    FixedBuffer() :
        cur_(data_)
    {

    }
    ~FixedBuffer()
    {

    }

    void Append(const char *buf, size_t len) // 添加数据到data
    {
        if(static_cast<size_t>(Avail()) > len)
        {
            memcpy(cur_, buf, len);
            cur_ += len;
        }
        else
        {
            std::cout << "Buffer size exceeded" << std::endl;
        }
    }
    const char* Data() const // 返回数据首地址
    {
        return data_;
    }
    int Length() const // 已写入的长度
    {
        return static_cast<int>(cur_ - data_);
    }
    char* Current() const // 获得当前位置指针
    {
        return cur_;
    }
    void Reset() // 重置当前指针
    {
        cur_ = data_;
    }
    void Bzero() // 将data置0
    {
        bzero(data_, sizeof(data_));
    }
    int Avail() const // 返回可写的字节数
    {
        return static_cast<int>(End() - cur_);
    }
private:
    char data_[SIZE];
    char *cur_; // 当前指针位置

    const char* End() const // 数据结尾指针，相当于容器的end()迭代器
    {
        return data_ + sizeof(data_);
    }
};

class LogStream
{
public:
    const static int SMALL_BUFFER = 4096;
    const static int LARGE_BUFFER = 4096 * 1024;
    using Buffer = FixedBuffer<SMALL_BUFFER>;

    LogStream();
    ~LogStream();

    const Buffer& FormatBuffer() const // 返回格式化的日志流
    {
        return buffer_;
    }
    void ResetBuffer() // 日志流重置
    {
        buffer_.Reset();
    }
    
    // 各种数据的格式化，使用snprintf，或许还可以用其他方法优化，如C20的format
    LogStream& operator<<(bool v)
    {
        buffer_.Append(v ? "1" : "0", 1);
        return *this;
    }
    LogStream& operator<<(char v)
    {
        buffer_.Append(&v, 1);
        return *this;
    }
    LogStream& operator<<(short v)
    {
        char buf[32];
        int n = snprintf(buf, 32, "%hd", v); // 效率比ostringstream高
        buffer_.Append(buf, n);
        return *this;
    }
    LogStream& operator<<(unsigned short v)
    {
        char buf[32];
        int n = snprintf(buf, 32, "%hu", v);
        buffer_.Append(buf, n);
        return *this;
    }
    LogStream& operator<<(int v)
    {
        char buf[32];
        int n = snprintf(buf, 32, "%d", v);
        buffer_.Append(buf, n);
        return *this;
    }
    LogStream& operator<<(unsigned int v)
    {
        char buf[32];
        int n = snprintf(buf, 32, "%u", v);
        buffer_.Append(buf, n);
        return *this;
    }
    LogStream& operator<<(long v)
    {
        char buf[32];
        int n = snprintf(buf, 32, "%ld", v);
        buffer_.Append(buf, n);
        return *this;
    }
    LogStream& operator<<(unsigned long v)
    {
        char buf[32];
        int n = snprintf(buf, 32, "%lu", v);
        buffer_.Append(buf, n);
        return *this;
    }
    LogStream& operator<<(long long v)
    {
        char buf[32];
        int n = snprintf(buf, 32, "%lld", v);
        buffer_.Append(buf, n);
        return *this;
    }
    LogStream& operator<<(unsigned long long v)
    {
        char buf[32];
        int n = snprintf(buf, 32, "%llu", v);
        buffer_.Append(buf, n);
        return *this;
    }
    LogStream& operator<<(float v)
    {
        char buf[32];
        int n = snprintf(buf, 32, "%f", v);
        buffer_.Append(buf, n);
        return *this;
    }
    LogStream& operator<<(double v)
    {
        char buf[32];
        int n = snprintf(buf, 32, "%lf", v);
        buffer_.Append(buf, n);
        return *this;
    }
    LogStream& operator<<(const char *str)
    {
        if(str != nullptr)
        {
            buffer_.Append(str, strlen(str));
        }
        else
        {
            buffer_.Append("(null)", 6);
        }
        return *this;
    }
    LogStream& operator<<(const unsigned char *str)
    {
        return operator<<(reinterpret_cast<const char*>(str));
    }
    LogStream& operator<<(const std::string& v)
    {
        buffer_.Append(v.c_str(), v.size());
        return *this;
    }
private:
    Buffer buffer_; // 日志流缓冲区
};

#endif
