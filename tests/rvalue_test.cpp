#include <iostream>
using namespace std;

template<typename T>
void PrintType(T&& t)
{

    if(is_same<T, int>::value)
    {
        cout << "int&& [rvalue] " << t << endl;
    }
    else if(is_same<T, int&>::value)
    {
        cout << "int& && [lvalue] " << t << endl;
    }
    else if(is_same<T, const int>::value)
    {
        cout << "const int && [const rvalue] " << t << endl;
    }
    else if(is_same<T, const int&>::value)
    {
        cout << "const int& && [const lvalue] " << t << endl;
    }
    else
    {
        cout << "unknown" << endl;
    }
}

template<typename T>
void Forward(T&& t)
{
    cout << "input-------";
    if(is_same<T, int>::value)
    {
        cout << "int&& [rvalue] " << t << endl;
    }
    else if(is_same<T, int&>::value)
    {
        cout << "int& && [lvalue] " << t << endl;
    }
    else if(is_same<T, const int>::value)
    {
        cout << "const int && [const rvalue] " << t << endl;
    }
    else if(is_same<T, const int&>::value)
    {
        cout << "const int& && [const lvalue] " << t << endl;
    }
    else
    {
        cout << "unknown" << endl;
    }
    cout << "not forward------";
    PrintType(t);
    cout << "std::forward------";
    PrintType(forward<T>(t));
    cout << "-----------end" << endl;
}

void PrintType2(int& t)
{
    cout << "int& " << endl;
}

void PrintType2(int&& t)
{
    cout << "int&& " << endl;
}

void Forward2(int&& t){
    cout << &t << endl;
    cout << "Forward2(int&&):" << endl;
    PrintType2(t);
}

int main()
{
    int a = 10;
    int& b = a;
    int&& c = 1;
    cout << "start***********0***" << endl;
    Forward(0);
    cout << "start***********int a = 10***" << endl;
    Forward(a);
    cout << "start***********move(a)***" << endl;
    Forward(move(a));
    cout << "start***********int& b = a***" << endl;
    Forward(b);
    cout << "start***********int&& c = 1***" << endl;
    Forward(c);
    const int d = 11;
    cout << "start***********const int d = 11***" << endl;
    Forward(d);
    cout << "start***********move(d)***" << endl;
    Forward(move(d));

    // Forward2(10); // wrong if have no PrintType2(int& t)，虽然在函数外部是个右值，但在函数内部看它是一个左值
    // Forward2(10);
    return 0;
}