/**
* @file     Logger.cpp
* @brief    日志类封装
* @author   lddddd (https://github.com/lddddd1997)
*/
#include <thread>
#include <sstream>
#include <assert.h>
#include "Logger.h"

Logger::Logger(const char *source_file, int line, LogLevelEnum level) :
    base_name_(source_file),
    line_(line),
    level_(level)
{
    FormatFrontPart();
    stream_ << LogLevelName[level_];
}

Logger::Logger(const char *source_file, int line, LogLevelEnum level, const char *func) :
    base_name_(source_file),
    line_(line),
    level_(level),
    time_(Timestamp::Now())
{
    FormatFrontPart();
    stream_ << LogLevelName[level];
    stream_ << func << " ";
}

Logger::OutputFunc g_output = Logger::DefaultOutput;
Logger::FlushFunc g_flush = Logger::defaultFlush;
Logger::LogLevelEnum g_log_level;

Logger::~Logger()
{
    FormatBackPart();
    g_output(stream_.FormatBuffer().Data(), stream_.FormatBuffer().Length());
    if(level_ == FATAL)
    {
        g_flush();
        abort();
    }
}

Logger::LogLevelEnum Logger::LogLevel()
{
    return g_log_level;
}

void Logger::SetLogLevel(Logger::LogLevelEnum level)
{
    g_log_level = level;
}

void Logger::SetOutput(OutputFunc out)
{
    g_output = out;
}

void Logger::SetFlush(FlushFunc flush)
{
    g_flush = flush;
}

void Logger::DefaultOutput(const char *log_line_, int len)
{
    size_t n = fwrite(log_line_, 1, len, stdout); // fwrite线程安全
    (void)n; // 去除警告
}

void Logger::defaultFlush()
{
    fflush(stdout);
}

thread_local time_t t_last_log_time = 0; // thread_local变量，每个线程有一份，与线程中的局部变量相似，但不用在各个方法内传递
thread_local char t_time_buf[32];

void Logger::FormatFrontPart()
{
    time_t seconds = static_cast<time_t>(time_.MicroSecondsSinceEpoch() / Timestamp::MICRO_SECONDS_PER_SECOND);
    int micro_seconds = static_cast<int>(time_.MicroSecondsSinceEpoch() % Timestamp::MICRO_SECONDS_PER_SECOND);
    // if(seconds != t_last_log_time)
    {
        t_last_log_time = seconds;
        struct tm tm_time;
        gmtime_r(&seconds, &tm_time);

        int len = snprintf(t_time_buf, sizeof(t_time_buf), "%4d%02d%02d %02d:%02d:%02d",
        tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
        tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
        assert(len == 17);
        t_time_buf[17] = '\0';
    }
    
    stream_ << t_time_buf;
    stream_ << "." << micro_seconds << " ";
    std::ostringstream oss; // 可能会影响效率
    oss << std::this_thread::get_id();
    stream_ << oss.str() << " ";
}

void Logger::FormatBackPart()
{ 
    stream_ << " - "<< base_name_ << ":" << line_ << '\n';
}
