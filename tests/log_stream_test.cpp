#include "../NetServer/LogStream.h"
#include <iostream>
#include <sstream>
#include <thread>
using namespace std;



int main()
{
    LogStream log_stream;
    // for(int i = 0; i < 120; i++)
    // {
    //     log_stream << "asdasd";
    //     log_stream << " ";
    //     log_stream << 123;
    //     log_stream << " ";
    //     log_stream << 456u;
    //     log_stream << " ";
    //     log_stream << 123.0f;
    //     log_stream << " ";
    //     log_stream << 456.0;
    // }
    std::ostringstream oss; // 可能会影响效率
    oss << std::this_thread::get_id();
    log_stream << oss.str();
    cout << std::this_thread::get_id() << endl;
    cout << log_stream.FormatBuffer().Data() << endl;

    return 0;
}
