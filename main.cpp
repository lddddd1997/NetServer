#include <thread_pool.h>
#include <iostream>
#include <unistd.h>
using namespace std;

ThreadPool Pool(10);
void test1()
{
    sleep(1);
}

void test2()
{
    for(int i = 0; i < 10; i++)
    {
        Pool.CommitTask(test1);
        usleep(rand() % 1000000);
    }
}

int main()
{
    
    Pool.Start();
    sleep(1);
    Pool.CommitTask(test2);
    for(int i = 0; i < 10; i++)
    {
        cout << Pool.IdleThreadCount() << " " << Pool.ThreadCount() << " " << Pool.TasksCount() << endl;
        sleep(1);
    }
    return 0;
}
