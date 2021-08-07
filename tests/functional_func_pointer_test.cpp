#include <functional>
#include <iostream>
#include <sys/time.h>
#include <unistd.h>
using namespace std;

const int MICRO_SECONDS_PER_SECOND = 1000000;

int64_t Now()
{
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    int64_t seconds = tv.tv_sec;
    return seconds * MICRO_SECONDS_PER_SECOND + tv.tv_usec;
}

int test(int a, int b)
{
    int c = a + b;
    int d = c / a;
    return d * c;
}

using FuncPtr = int (*)(int, int);
using Function = function<int(int, int)>;

int main()
{
    FuncPtr func_ptr = test;
    Function func = test;
    sleep(1);
    int64_t t1 = Now();
    for(int i = 0; i < 1e9; i++)
        test(2, 4);
    cout << "direct call: " << Now() - t1 << endl;

    t1 = Now();
    for(int i = 0; i < 1e9; i++)
        func_ptr(2, 4);
    cout << "func_ptr call: " << Now() - t1 << endl;

    t1 = Now();
    for(int i = 0; i < 1e9; i++)
        func(2, 4);
    cout << "function call: " << Now() - t1 << endl;
    return 0;
}

