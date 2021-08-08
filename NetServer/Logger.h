/**
* @file     Logger.h
* @brief    日志类封装
* @author   lddddd (https://github.com/lddddd1997)
*/
#ifndef LOGGER_H_
#define LOGGER_H_

#include <atomic>
#include "LogStream.h"
#include "Timestamp.h"

#define LOG_TRACE \
        if(Logger::LogLevel() <= Logger::TRACE) \
            Logger(__FILE__, __LINE__, Logger::TRACE, __func__).Stream()
#define LOG_DEBUG \
        if(Logger::LogLevel() <= Logger::DEBUG) \
            Logger(__FILE__, __LINE__, Logger::DEBUG, __func__).Stream()
#define LOG_INFO \
        if(Logger::LogLevel() <= Logger::INFO) \
            Logger(__FILE__, __LINE__, Logger::INFO).Stream()
#define LOG_WARN Logger(__FILE__, __LINE__, Logger::WARN).Stream()
#define LOG_ERROR Logger(__FILE__, __LINE__, Logger::ERROR).Stream()
#define LOG_FATAL Logger(__FILE__, __LINE__, Logger::FATAL).Stream()


class Logger
{
public:
    using OutputFunc = void(*)(const char *log_line_, int len); // 函数指针效率比function高得多
    using FlushFunc = void(*)();
    enum LogLevelEnum
    {
        TRACE,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL,
        NUM_LOG_LEVELS
    };
    Logger(const char *source_file, int line, LogLevelEnum level);
    Logger(const char *source_file, int line, LogLevelEnum level, const char *func);
    ~Logger();

    static LogLevelEnum LogLevel();
    static void SetLogLevel(LogLevelEnum level);
    static void SetOutput(OutputFunc out);
    static void SetFlush(FlushFunc flush);
    static void DefaultOutput(const char *log_line_, int len);
    static void defaultFlush();

    LogStream& Stream()
    {
        return stream_;
    }
private:
    const char *LogLevelName[Logger::NUM_LOG_LEVELS] =
    {
        "TRACE ",
        "DEBUG ",
        "INFO  ",
        "WARN  ",
        "ERROR ",
        "FATAL ",
    };
    const char *base_name_;
    int line_;
    LogLevelEnum level_;
    Timestamp time_;
    LogStream stream_;

    void FormatFrontPart();
    void FormatBackPart();
};

#endif
