#include "../NetServer/LogFile.h"
#include <iostream>
#include <unistd.h>
using namespace std;

int main()
{
    LogFile log_file("log_test", 2 * 1000);
    for(int i = 0; i < 100000; i++)
    {
        log_file.AppendUnlocked("978654312", 10);
        log_file.AppendUnlocked("123456789", 10);
        sleep(1);
    }
    return 0;
}

