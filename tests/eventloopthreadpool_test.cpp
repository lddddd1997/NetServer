#include "../NetServer/EventLoopThreadPool.h"
#include <iostream>
#include <unistd.h>
#include <assert.h>
using namespace std;

// g++ eventloopthreadpool_test.cpp ../obj/*.o -pthread

// /usr/bin/valgrind --tool=memcheck --leak-check=full --log-file=log.txt ./a.out

/*
==7011== Memcheck, a memory error detector
==7011== Copyright (C) 2002-2017, and GNU GPL'd, by Julian Seward et al.
==7011== Using Valgrind-3.13.0 and LibVEX; rerun with -h for copyright info
==7011== Command: ./a.out
==7011== Parent PID: 2846
==7011== 
==7011== 
==7011== HEAP SUMMARY:
==7011==     in use at exit: 0 bytes in 0 blocks
==7011==   total heap usage: 152 allocs, 152 frees, 80,192 bytes allocated
==7011== 
==7011== All heap blocks were freed -- no leaks are possible
==7011== 
==7011== For counts of detected and suppressed errors, rerun with: -v
==7011== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)
*/

void print(EventLoop *p = nullptr)
{
    printf("print: pid = %d, tid = %ld, loop = %p\n",
        getpid(), this_thread::get_id(), p);
}

int main()
{
    EventLoop basic_loop("basic loop");
    {
        EventLoopThreadPool Pool(&basic_loop, 0);
        Pool.Start();
        EventLoop *loop = Pool.GetNextLoop();
        loop->CommitTaskToLoop(bind(print, loop));
        assert(loop == &basic_loop);
        assert(Pool.GetNextLoop() == &basic_loop);
        assert(Pool.GetNextLoop() == &basic_loop);
        assert(Pool.GetNextLoop() == &basic_loop);
    }

    {
        EventLoop basic_loop("basic loop");
        EventLoopThreadPool Pool(&basic_loop, 3);
        Pool.Start();
        EventLoop *loop = Pool.GetNextLoop();
        loop->CommitTaskToLoop(bind(print, loop));
        assert(loop != &basic_loop);
        assert(Pool.GetNextLoop() != loop);
        EventLoop *loop1 = Pool.GetNextLoop();
        loop1->CommitTaskToLoop(bind(print, loop1));
        assert(loop1 != loop);
        assert(Pool.GetNextLoop() == loop);
    }
    cout << "-------------------------------" << endl << endl;
    EventLoopThreadPool Pool(&basic_loop, 2);
    Pool.Start();
    for(int i = 0; i < 5; i++)
    {
        EventLoop *loop = Pool.GetNextLoop();
        loop->CommitTaskToLoop(bind(print, loop));
        loop->CommitTaskToLoop(bind(print, loop));
        loop->CommitTaskToLoop(bind(print, loop));
        loop->CommitTaskToLoop(bind(print, loop));
        cout << loop->ThreadId() << endl;
        sleep(1);
    }

    return 0;
}
