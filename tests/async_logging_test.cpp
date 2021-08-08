#include "../NetServer/AsyncLogging.h"
#include <iostream>
#include <unistd.h>
#include <sys/time.h>
using namespace std;

int main()
{
    AsyncLogging log("log_test", 5 * 1024 * 1024);
    log.Start();
    string str(3000, 'x');
    str += " ";
    for(int i = 0; i < 4096 * 1024; i++)
    {
        // struct timeval tv1;
        // gettimeofday(&tv1, nullptr);
        log.Append("0978654321", 10);
        log.Append("1234567890", 10);
        // log.Append(str.c_str(), sizeof(str));
        usleep(10);
        // struct timeval tv2;
        // gettimeofday(&tv2, nullptr);
        // cout << tv2.tv_usec - tv1.tv_usec << endl;
    }
    std::cout << "Finished" << std::endl;
    while(1);
    return 0;
}



