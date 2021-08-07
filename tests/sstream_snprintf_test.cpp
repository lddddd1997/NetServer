#include <sstream>
#include <iostream>
#include <sys/time.h>
#include <unistd.h>
#include <cstring>
using namespace std;

const int MICRO_SECONDS_PER_SECOND = 1000000;

const int TEST_COUNT = 100000;

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
        oss << "qwert";
		// oss << (i * 1.0);
        // oss << i;
	}
    cout << "test_stringstream1: " << Now() - t1 << endl;
}

void test_stringstream2()
{
    int64_t t1 = Now();
	stringstream oss;
	for(int i = 0 ; i < TEST_COUNT; i++)
	{
		oss.clear();
		oss.str("");
        oss << "qwert";
		// oss << (i * 1.0);
        // oss << i;
	}
    cout << "test_stringstream2: " << Now() - t1 << endl;
}

void test_snprintf1()
{
    int64_t t1 = Now();
	for(int i = 0 ; i < TEST_COUNT; i++)
	{
		char buf[1024];
        snprintf(buf, sizeof(buf), "%s", "qwert");
		// snprintf(buf, sizeof(buf), "%f", (i * 1.0));
        // snprintf(buf, sizeof(buf), "%f", i);
	}
    cout << "test_snprintf1: " << Now() - t1 << endl;
}
void test_snprintf2()
{
    int64_t t1 = Now();
	char buf[1024];
	for(int i = 0 ; i < TEST_COUNT; i++)
	{
        bzero(buf, 1024);
        snprintf(buf, sizeof(buf), "%s", "qwert");
		// snprintf(buf, sizeof(buf), "%f", (i * 1.0));
        // snprintf(buf, sizeof(buf), "%f", i);
	}
    cout << "test_snprintf2: " << Now() - t1 << endl;
}

int main()
{
    sleep(1);
    test_stringstream1();
    test_stringstream2();
    test_snprintf1();
    test_snprintf2();

    return 0;
}

