#include <sstream>
#include <iostream>
#include <sys/time.h>
#include <unistd.h>
#include <cstring>
#include "../NetServer/LogStream.h"
using namespace std;

// g++ sstream_snprintf_test.cpp ../NetServer/LogStream.cpp
const int MICRO_SECONDS_PER_SECOND = 1000000;

const int TEST_COUNT = 1000000;

enum TestType
{
    DOUBLE_T,
    INT_T,
    CHAR_BUF_T
};

TestType test_type = CHAR_BUF_T;

int64_t Now()
{
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    int64_t seconds = tv.tv_sec;
    return seconds * MICRO_SECONDS_PER_SECOND + tv.tv_usec;
}

void test_stringstream1()
{
    int64_t t1 = Now();
	for(int i = 0 ; i < TEST_COUNT; i++)
	{
		stringstream oss;
        if(test_type == CHAR_BUF_T)
            oss << "qwert";
        else if(test_type == DOUBLE_T)
		    oss << (i * 1.0);
        else if(test_type == INT_T)
            oss << i;
	}
    cout << "test_stringstream1: " << Now() - t1 << "ms" << endl;
}

void test_stringstream2()
{
    int64_t t1 = Now();
	stringstream oss;
	for(int i = 0 ; i < TEST_COUNT; i++)
	{
		oss.clear();
		oss.str("");
        if(test_type == CHAR_BUF_T)
            oss << "qwert";
        else if(test_type == DOUBLE_T)
		    oss << (i * 1.0);
        else if(test_type == INT_T)
            oss << i;
	}
    cout << "test_stringstream2: " << Now() - t1 << "ms" << endl;
}

void test_snprintf1()
{
    int64_t t1 = Now();
	for(int i = 0 ; i < TEST_COUNT; i++)
	{
		char buf[1024];
        if(test_type == CHAR_BUF_T)
            snprintf(buf, sizeof(buf), "%s", "qwert");
        else if(test_type == DOUBLE_T)
		    snprintf(buf, sizeof(buf), "%f", (i * 1.0));
        else if(test_type == INT_T)
            snprintf(buf, sizeof(buf), "%d", i);
	}
    cout << "test_snprintf1: " << Now() - t1 << "ms" << endl;
}
void test_snprintf2()
{
    int64_t t1 = Now();
	char buf[1024];
	for(int i = 0 ; i < TEST_COUNT; i++)
	{
        bzero(buf, 1024);
        if(test_type == CHAR_BUF_T)
            snprintf(buf, sizeof(buf), "%s", "qwert");
        else if(test_type == DOUBLE_T)
		    snprintf(buf, sizeof(buf), "%f", (i * 1.0));
        else if(test_type == INT_T)
            snprintf(buf, sizeof(buf), "%d", i);
	}
    cout << "test_snprintf2: " << Now() - t1 << "ms" << endl;
}

void test_logstream1()
{
    int64_t t1 = Now();
	for(int i = 0 ; i < TEST_COUNT; i++)
	{
        LogStream log_stream;
        if(test_type == CHAR_BUF_T)
            log_stream << "qwert";
        else if(test_type == DOUBLE_T)
		    log_stream << (i * 1.0);
        else if(test_type == INT_T)
            log_stream << i;
	}
    cout << "test_logstream: " << Now() - t1 << "ms" << endl;
}

void test_logstream2()
{
    int64_t t1 = Now();
    LogStream log_stream;
	for(int i = 0 ; i < TEST_COUNT; i++)
	{
        log_stream.ResetBuffer();
        if(test_type == CHAR_BUF_T)
            log_stream << "qwert";
        else if(test_type == DOUBLE_T)
		    log_stream << (i * 1.0);
        else if(test_type == INT_T)
            log_stream << i;
	}
    cout << "test_logstream: " << Now() - t1 << "ms" << endl;
}

int main()
{
    sleep(1);
    test_stringstream1();
    test_stringstream2();
    test_snprintf1();
    test_snprintf2();
    test_logstream1();
    test_logstream2();
    return 0;
}

