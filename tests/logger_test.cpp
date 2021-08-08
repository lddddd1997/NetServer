#include "../NetServer/Logger.h"
#include <iostream>
using namespace std;
// g++ logger_test.cpp ../NetServer/Logger.cpp ../NetServer/Timestamp.cpp ../NetServer/LogStream.cpp
int main()
{
    // Logger::SetLogLevel(Logger::LogLevelEnum::ERROR);
    LOG_TRACE << 123.0;
    LOG_DEBUG << "asd";
    LOG_INFO << 456.0;
    LOG_WARN << 789;
    LOG_ERROR << "error";
    LOG_FATAL << "fatal";
    return 0;
}

