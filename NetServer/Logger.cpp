/**
* @file     Logger.cpp
* @brief    日志类封装
* @author   lddddd (https://github.com/lddddd1997)
* TODO:     优化线程id的格式化
*/
#include <thread>
#include <sstream>
#include <assert.h>
#include "Logger.h"

const char *Logger::LogLevelName[Logger::NUM_LOG_LEVELS] =
    {
        "TRACE ",
        "DEBUG ",
        "INFO  ",
        "WARN  ",
        "ERROR ",
        "FATAL ",
    };
std::unique_ptr<AsyncLogging> Logger::async_logging_ = nullptr;
Logger::OutputFunc Logger::log_output_ = Logger::DefaultOutput;
Logger::FlushFunc Logger::log_flush_ = Logger::defaultFlush;
Logger::LogLevelEnum Logger::log_level_ = Logger::TRACE;


Logger::Logger() :
    base_name_(__FILE__),
    line_(__LINE__),
    level_(Logger::INFO),
    time_(Timestamp::Now())
{
    // SetLogLevel(Logger::TRACE);
    FormatFrontPart();
    stream_ << Logger::LogLevelName[level_];
}

Logger::Logger(const char *source_file, int line, Logger::LogLevelEnum level) :
    base_name_(source_file),
    line_(line),
    level_(level),
    time_(Timestamp::Now())
{
    FormatFrontPart();
    stream_ << Logger::LogLevelName[level_];
}

Logger::Logger(const char *source_file, int line, Logger::LogLevelEnum level, const char *func) :
    base_name_(source_file),
    line_(line),
    level_(level),
    time_(Timestamp::Now())
{
    FormatFrontPart();
    stream_ << Logger::LogLevelName[level];
    stream_ << func << " ";
}

Logger::~Logger()
{
    FormatBackPart();
    log_output_(stream_.FormatBuffer().Data(), stream_.FormatBuffer().Length());
    if(level_ == Logger::FATAL)
    {
        log_flush_();
        abort();
    }
}

void Logger::SetOutput(OutputFunc out)
{
    log_output_ = out;
}

void Logger::SetFlush(FlushFunc flush)
{
    log_flush_ = flush;
}

thread_local time_t t_last_log_time = 0; // thread_local变量，每个线程有一份，与线程中的局部变量相似，但不用在各个方法内传递
thread_local char t_time_buf[32];

void Logger::FormatFrontPart()
{
    time_t seconds = static_cast<time_t>(time_.MicroSecondsSinceEpoch() / Timestamp::MICRO_SECONDS_PER_SECOND);
    int micro_seconds = static_cast<int>(time_.MicroSecondsSinceEpoch() % Timestamp::MICRO_SECONDS_PER_SECOND);
    if(seconds != t_last_log_time) // 一秒内只更新微秒部分
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
    char ms_buf[16];
    snprintf(ms_buf, sizeof(ms_buf), ".%06dZ ", micro_seconds);
    stream_ << ms_buf;
    std::ostringstream oss; // 特别影响效率，待优化
    oss << std::this_thread::get_id();
    stream_ << oss.str() << " ";
}

void Logger::FormatBackPart()
{ 
    stream_ << " - "<< base_name_ << ":" << line_ << '\n';
}
