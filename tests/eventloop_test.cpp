// #include <EventLoopThread.h>
// #include <iostream>
// #include <unistd.h>
// using namespace std;

// // g++ eventloop_test.cpp EventLoopThread.cpp EventLoop.cpp Channel.cpp Epoller.cpp -I . -pthread

// void print(EventLoop *p = nullptr)
// {
//     printf("print: pid = %d, tid = %ld, loop = %p\n",
//         getpid(), this_thread::get_id(), p);
// }

// void quit(EventLoop* p)
// {
//   print(p);
//   p->CommitTaskToLoop(std::bind(print, p));
// //   p->Quit();
// }

// int main()
// {
//     print();

//     {
//         EventLoopThread thr1;  // never start
//     }
//     // {
//     //     // dtor calls quit()
//     //     EventLoopThread thr2;
//     //     EventLoop* loop = thr2.StartLoop();
//     //     loop->CommitTaskToLoop(std::bind(print, loop));
//     //     // sleep(5);
//     // }

//     {
//         // quit() before dtor
//         EventLoopThread thr3;
//         EventLoop* loop = thr3.StartLoop();
//         loop->CommitTaskToLoop(std::bind(quit, loop));
//         sleep(5);
//     }

//     return 0;
// }
