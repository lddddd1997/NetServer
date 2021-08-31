#include <iostream>
#include <unordered_map>
using namespace std;

int main()
{
    unordered_map<int, int> mp;
    for(int i = 0; i < 17; i++)
    {
        mp[i] = i;
        cout << i + 1 << " " << mp.bucket_count() << endl;
    }
    return 0;
}
