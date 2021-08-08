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

    void Append(const char *logline, size_t len);
    void Flush();
    off_t WrittenBytes() const
    {
        return written_bytes_;
    }

private:
    FILE* fp_;
    char buffer_[64 * 1024];
    off_t written_bytes_;
    size_t Write(const char *logline, size_t len);
};

class LogFile
{
public:
    LogFile(const std::string& base_name, off_t roll_size, int flush_interval = 3, int check_every_count = 1024);
    ~LogFile();

    void AppendUnlocked(const char *logline, int len);
    void Flush();

private:
    const std::string base_name_;
    const off_t roll_size_;
    const int flush_interval_;
    const int check_every_count_;
    int count_;
    time_t start_of_period_;
    time_t last_roll_;
    time_t last_flush_;
    std::unique_ptr<AppendFile> file_;
    const static int ROLL_PER_SECONDS = 60 * 60 * 24;
    
    bool RollFile();
    std::string GetLogFileName(time_t *now);
};

#endif
