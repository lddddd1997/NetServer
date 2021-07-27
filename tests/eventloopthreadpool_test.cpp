// #include <EventLoopThreadPool.h>
// #include <iostream>
// #include <unistd.h>
// #include <assert.h>
// using namespace std;

// //g++ eventloopthreadpool_test.cpp EventLoopThreadPool.cpp EventLoopThread.cpp EventLoop.cpp Channel.cpp Epoller.cpp -I . -pthread
// void print(EventLoop *p = nullptr)
// {
//     printf("print: pid = %d, tid = %ld, loop = %p\n",
//         getpid(), this_thread::get_id(), p);
// }

// int main()
// {
//     EventLoop base_loop;
//     {
//         EventLoopThreadPool Pool(&base_loop, 0);
//         Pool.Start();
//         EventLoop *loop = Pool.GetNextLoop();
//         loop->CommitTaskToLoop(bind(print, loop));
//         assert(loop == &base_loop);
//         assert(Pool.GetNextLoop() == &base_loop);
//         assert(Pool.GetNextLoop() == &base_loop);
//         assert(Pool.GetNextLoop() == &base_loop);
//     }

//     {
//         EventLoop base_loop;
//         EventLoopThreadPool Pool(&base_loop, 3);
//         Pool.Start();
//         EventLoop *loop = Pool.GetNextLoop();
//         loop->CommitTaskToLoop(bind(print, loop));
//         assert(loop != &base_loop);
//         assert(Pool.GetNextLoop() != loop);
//         EventLoop *loop1 = Pool.GetNextLoop();
//         loop1->CommitTaskToLoop(bind(print, loop1));
//         assert(loop1 != loop);
//         assert(Pool.GetNextLoop() == loop);
//     }
//     cout << "-------------------------------" << endl << endl;
//     EventLoopThreadPool Pool(&base_loop, 2);
//     Pool.Start();
//     for(int i = 0; i < 5; i++)
//     {
//         EventLoop *loop = Pool.GetNextLoop();
//         loop->CommitTaskToLoop(bind(print, loop));
//         loop->CommitTaskToLoop(bind(print, loop));
//         loop->CommitTaskToLoop(bind(print, loop));
//         loop->CommitTaskToLoop(bind(print, loop));
//         cout << loop->ThreadId() << endl;
//         sleep(1);
//     }

//     return 0;
// }
