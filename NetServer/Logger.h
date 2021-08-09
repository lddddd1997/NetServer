/**
* @file     AsyncLogging.h
* @brief    日志类封装
* @author   lddddd (https://github.com/lddddd1997)
*/
#ifndef LOGGER_H_
#define LOGGER_H_

#include "AsyncLogging.h"
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
    using OutputFunc = void(*)(const char *log_line, int len); // 函数指针效率比function高得多
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

    Logger(); // 用于实例化调用设置Logger方式，default或async
    Logger(const char *source_file, int line, LogLevelEnum level);
    Logger(const char *source_file, int line, LogLevelEnum level, const char *func);
    ~Logger();

    void EnableAsyncLogging(const std::string& base_name, int roll_size) // 使能异步日志
    {
        if(async_logging_ == nullptr)
        {
            async_logging_.reset(new AsyncLogging(base_name, roll_size));
        }
        else
        {
            async_logging_.reset(new AsyncLogging(base_name, roll_size));
        }
        async_logging_->Start();
        SetOutput(AyncOutput);

        stream_ << __func__ << " ";
        stream_ << "Enable asynclogging";
    }
    void DisableAsyncLogging() // 失能异步日志，使用默认日志输出
    {
        // async_logging_.reset(nullptr); // 存在线程安全，宁愿错写也不希望发生访问空指针
        SetOutput(DefaultOutput);
        // async_logging_->Stop();

        stream_ << __func__ << " ";
        stream_ << "Disable asynclogging";
    }
    void SetLogLevel(LogLevelEnum level) // 设置全局的日志级别
    {
        log_level_ = level;
    }
    static LogLevelEnum LogLevel() // 返回全局的日志级别
    {
        return log_level_;
    }
    LogStream& Stream()
    {
        return stream_;
    }
private:
    const char *base_name_; // 前缀名
    int line_; // 行号
    LogLevelEnum level_; // 局部的日志级别
    Timestamp time_; // 局部的时间戳
    LogStream stream_; // 局部的日志流
    
    static const char *LogLevelName[Logger::NUM_LOG_LEVELS];
    static std::unique_ptr<AsyncLogging> async_logging_; // 全局的异步日志对象，默认为nullptr
    static OutputFunc log_output_; // 全局的日志输出方式，目前可设置异步EnableAsyncLogging或默认DisableAsyncLogging
    static FlushFunc log_flush_; // 全局的日志刷新
    static LogLevelEnum log_level_; // 全局的日志级别，默认为Logger::TRACE，可使用SetLogLevel接口进行设置

    static void DefaultOutput(const char *log_line, int len) // 默认输出方式
    {
        size_t n = fwrite(log_line, 1, len, stdout); // fwrite线程安全
        (void)n; // 去除警告
    }
    static void defaultFlush() // 默认刷新方式
    {
        fflush(stdout);
    }
    static void AyncOutput(const char *log_line, int len) // 使用异步日志输出方式，存在线程安全，但几乎只设置一次
    {
        if(async_logging_ != nullptr)
        {
            async_logging_->Append(log_line, len);
        }
        else
        {
            std::cout << "async_logging_ was null" << std::endl;
        }
    }
    void SetOutput(OutputFunc out); // 设置日志流的输出方式，默认输出到stdout
    void SetFlush(FlushFunc flush); // 设置日志流的刷新方式，默认刷新stdout
    void FormatFrontPart(); // 格式化日志数据的前半部分
    void FormatBackPart(); // 格式化日志数据的后半部分
};

#endif
