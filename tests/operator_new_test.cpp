#include <iostream>
using namespace std;

class ADT{
public:
	ADT(){
		i = 10;
		j = 100;
		cout << "ADT construct i=" << i << " j="<<j <<endl;
	}
	~ADT(){
		cout << "ADT destruct" << endl;
	}
private:
    int i;
	int j;
};


int main(int argc, char *argv[])
{
    // int *p = new(nothrow) int[99999999999];
	// if (p == nullptr) 
	// {
	// 	cout << "alloc failed" << endl;
	// }
	// delete p;
    
    // try
	// {
	// 	int *p = new int[99999999999];
	// 	delete p;
	// }
	// catch (const std::bad_alloc &ex)
	// {
	// 	cout << ex.what() << endl;
	// }

    char *p = new(nothrow) char[sizeof(ADT) + 1];
	if (p == NULL) {
		cout << "alloc failed" << endl;
	}
	ADT *q = new(p) ADT; // placement new:不必担心失败，只要p所指对象的的空间足够ADT创建即可
	//delete q; // 错误!不能在此处调用delete q;
	q->ADT::~ADT(); // 显示调用析构函数
	delete[] p;

    return 0;
}