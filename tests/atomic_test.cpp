#include <iostream>
#include <memory>
#include <set>
#include <string>
#include <functional>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <vector>
#include <string>
#include <stdio.h>
#include <stdarg.h>
#include <wchar.h>
#include <cstring>
#include <sstream>
#include <atomic>
#include <thread>
#include <mutex>
using namespace std;

int num;
mutex mtx;
void test(int i)
{
    while(1)
    {
        {
        //lock_guard<mutex> lock(mtx);
        num = i;
        cout << endl << endl << "thread:  " << i << "       num = " << num << endl << endl;
        }
        usleep(rand() % 1000);
    }
}

const int N = 100;
int main(int argc, char *argv[])
{
    thread th[N];
    for(int i = 0; i < N; i++)
        th[i] = thread(test, i);

    while(1);
    return 0;
}