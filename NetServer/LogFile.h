/**
* @file     LogFile.h
* @brief    日志文件
* @author   lddddd (https://github.com/lddddd1997)
*/
#ifndef LOG_FILE_H_
#define LOG_FILE_H_

#include <memory>
#include <string>

class AppendFile
{
public:
    explicit AppendFile(const std::string& file_name);
    ~AppendFile();

    void Append(const char *logline, size_t len); // 文件写入
    void Flush(); // 文件刷新
    off_t WrittenBytes() const
    {
        return written_bytes_;
    }

private:
    FILE *fp_; // 文件指针
    char buffer_[64 * 1024]; // 文件缓冲区
    off_t written_bytes_; // 已写入的字节数
    size_t Write(const char *logline, size_t len); // 文件写入封装
};

class LogFile
{
public:
    LogFile(const std::string& base_name, off_t roll_size, int flush_interval = 3, int check_every_count = 1024);
    ~LogFile();

    void AppendUnlocked(const char *logline, int len); // 不加锁的文件写入
    void Flush();  // 文件刷新

private:
    const std::string base_name_; // 日志前缀名
    const off_t roll_size_; // 滚动日志大小
    const int flush_interval_; // 刷新周期
    const int check_every_count_; // 超过一定次数检查周期是否更新
    int count_; // 计数检查
    time_t start_of_period_; // 起始周期
    time_t last_roll_; // 上次滚动时间
    time_t last_flush_; // 上次刷新时间
    std::unique_ptr<AppendFile> file_; // 当前写入的日志文件
    const static int ROLL_PER_SECONDS = 60 * 60 * 24; // 每周期滚动日志文件
    
    bool RollFile(); // 滚动日志文件
    std::string FormatLogFileName(time_t *now); // 格式化Log文件名，base_name.time.hostname.pid.log
};

#endif
