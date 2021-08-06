#include "../NetServer/TimerQueue.h"
#include "../NetServer/Timestamp.h"
#include "../NetServer/EventLoop.h"
#include <iostream>
using namespace std;
// g++ -o timer_test timer_test.cpp ../obj/*.o -I ../NetServer/ -pthread
void print()
{
    cout << "test" << endl;
    cout << Timestamp::Now().MicroSecondsSinceEpoch() << endl;
}

void print_every()
{
    cout << Timestamp::Now().MicroSecondsSinceEpoch() << endl;
}


int main()
{
    EventLoop loop("test");

    loop.RunAfter(1, bind(&print));
    loop.RunAfter(1, bind(&print));
    loop.RunAfter(1, bind(&print));
    loop.RunAfter(1, bind(&print));
    loop.RunAfter(1.5, bind(&print));
    loop.RunAfter(2, bind(&print));
    loop.RunEvery(3, bind(&print_every));
    loop.RunEvery(5, bind(&print_every));
    loop.Looping();

    return 0;
}