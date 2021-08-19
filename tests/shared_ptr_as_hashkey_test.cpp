#include <iostream>
#include <memory>
#include <unordered_set>
using namespace std;

int main(int argc, char *argv[])
{
    shared_ptr<int> a(new int(10));
    shared_ptr<int> b(new int(10));
    shared_ptr<int> sp1 = a;
    shared_ptr<int> sp2 = a;
    unordered_set<shared_ptr<int>> s;
    s.insert(sp1);
    s.insert(sp2);
    cout << s.size() << endl;
    return 0;
}