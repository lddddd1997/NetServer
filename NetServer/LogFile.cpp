/**
* @file     LogFile.cpp
* @brief    日志文件
* @author   lddddd (https://github.com/lddddd1997)
*/
#include <assert.h>
#include <sys/time.h>
#include "Utilities.h"
#include "LogFile.h"

AppendFile::AppendFile(const std::string& file_name) :
    fp_(fopen(file_name.c_str(), "ae")),
    written_bytes_(0)
{
    assert(fp_);
    setbuffer(fp_, buffer_, sizeof(buffer_)); // 设置文件流的缓冲区
}

AppendFile::~AppendFile()
{
    fclose(fp_);
}

void AppendFile::Append(const char *logline, size_t len)
{
    size_t written = 0;
    while(written != len)
    {
        size_t remain = len - written;
        size_t n = Write(logline + written, remain);
        if (n != remain)
        {
            int err = ferror(fp_);
            if (err)
            {
                fprintf(stderr, "AppendFile::append() failed\n");
                break;
            }
        }
        written += n;
    }
    written_bytes_ += written;
}

void AppendFile::Flush()
{
    fflush(fp_);
}

size_t AppendFile::Write(const char *logline, size_t len)
{
    return fwrite_unlocked(logline, 1, len, fp_); // fwrite线程安全,但效率低
}

LogFile::LogFile(const std::string& base_name, off_t roll_size, int flush_interval, int check_every_count) :
    base_name_(base_name),
    roll_size_(roll_size),
    flush_interval_(flush_interval),
    check_every_count_(check_every_count),
    count_(0),
    start_of_period_(0),
    last_roll_(0),
    last_flush_(0)
{
    assert(base_name_.find('/') == std::string::npos);
    RollFile();
}

LogFile::~LogFile()
{

}

void LogFile::AppendUnlocked(const char *logline, int len)
{
    file_->Append(logline, len);
    if(file_->WrittenBytes() > roll_size_)
    {
        RollFile();
    }
    else
    {
        ++count_;
        if(count_ >= check_every_count_)
        {
            count_ = 0;
            time_t now = time(nullptr);
            time_t this_period = now / ROLL_PER_SECONDS * ROLL_PER_SECONDS; // 去除小于该周期的秒数，即每过一周期+ROLL_PER_SECONDS
            if(this_period != start_of_period_)
            {
                RollFile();
            }
            else if(now - last_flush_ > flush_interval_)
            {
                last_flush_ = now;
                file_->Flush();
            }
        }
    }
}

void LogFile::Flush()
{
    file_->Flush();
}

bool LogFile::RollFile()
{
    time_t now = 0;
    std::string file_name = GetLogFileName(&now); // 由于now的时间精度为s，如果前端发送太快，则文件名与上一秒的一致，则不会创建文件
    time_t start = now / ROLL_PER_SECONDS * ROLL_PER_SECONDS;

    if(now > last_roll_) // 由于now的时间精度为s，如果前端发送太快，则不会创建文件
    {
        last_roll_ = now;
        last_flush_ = now;
        start_of_period_ = start;
        file_.reset(new AppendFile(file_name));
        return true;
    }
    return false;
}

std::string LogFile::GetLogFileName(time_t *now)
{
    std::string file_name;
    file_name.reserve(base_name_.size() + 64);
    file_name = base_name_;

    char time_buf[32];
    struct tm tm;
    *now = time(nullptr); // 精度为s
    gmtime_r(now, &tm); // UTC时间，未转化成本地时间（localtime函数为本地时间，但效率差）
    strftime(time_buf, sizeof(time_buf), ".%Y%m%d-%H%M%S.", &tm);
    file_name += time_buf;

    file_name += Utilities::Hostname();

    char pidbuf[32];
    snprintf(pidbuf, sizeof(pidbuf), ".%d", Utilities::Pid());
    file_name += pidbuf;

    file_name += ".log";

    return file_name;
}
