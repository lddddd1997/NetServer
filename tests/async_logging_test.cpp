#include "../NetServer/AsyncLogging.h"
#include <iostream>
#include <unistd.h>
#include <sys/time.h>
using namespace std;

// g++ async_logging_test.cpp ../obj/*.o -pthread

// /usr/bin/valgrind --tool=memcheck --leak-check=full --log-file=log.txt ./a.out

/*
==7191== Memcheck, a memory error detector
==7191== Copyright (C) 2002-2017, and GNU GPL'd, by Julian Seward et al.
==7191== Using Valgrind-3.13.0 and LibVEX; rerun with -h for copyright info
==7191== Command: ./a.out
==7191== Parent PID: 2846
==7191== 
==7191== 
==7191== HEAP SUMMARY:
==7191==     in use at exit: 0 bytes in 0 blocks
==7191==   total heap usage: 57 allocs, 57 frees, 84,174,107 bytes allocated
==7191== 
==7191== All heap blocks were freed -- no leaks are possible
==7191== 
==7191== For counts of detected and suppressed errors, rerun with: -v
==7191== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)
*/
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
        log.Append("978654321\n", 10);
        log.Append("123456789\n", 10);
        // log.Append(str.c_str(), sizeof(str));
        // usleep(10);
        // struct timeval tv2;
        // gettimeofday(&tv2, nullptr);
        // cout << tv2.tv_usec - tv1.tv_usec << endl;
    }
    log.Stop();
    std::cout << "Finished" << std::endl;
    sleep(5);
    return 0;
}



