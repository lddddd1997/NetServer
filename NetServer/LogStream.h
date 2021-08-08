/**
* @file     LogStream.h
* @brief    日志流封装
* @author   lddddd (https://github.com/lddddd1997)
* TODO:     使用c++20的format替代snprintf的格式化
*/
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

    void Append(const char *buf, size_t len)
    {
        if(static_cast<size_t>(Avail()) > len)
        {
            memcpy(cur_, buf, len);
            cur_ += len;
        }
        else
        {
            std::cout << "Exceed the single write length" << std::endl;
        }
    }
    const char* Data() const
    {
        return data_;
    }
    int Length() const
    {
        return static_cast<int>(cur_ - data_);
    }
    char* Current() const
    {
        return cur_;
    }
    void Reset()
    {
        cur_ = data_;
    }
    void Bzero()
    {
        bzero(data_, sizeof(data_));
    }
    int Avail() const
    {
        return static_cast<int>(End() - cur_);
    }
private:
    char data_[SIZE];
    char *cur_;

    const char* End() const
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

    const Buffer& FormatBuffer() const
    {
        return buffer_;
    }
    void ResetBuffer()
    {
        buffer_.Reset();
    }
    
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
    Buffer buffer_;
};
