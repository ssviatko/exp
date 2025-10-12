#include <iostream>
#include <exception>

#include <stdlib.h>

using namespace std;

class demo {
public:
	demo()
	: m_val(-55)
	{ }
	demo(int a_val)
	: m_val(a_val)
	{ }
	~demo() { }
	void *operator new(size_t a_size) throw (bad_alloc);
	void *operator new[](size_t a_size) throw (bad_alloc);
	void operator delete(void *a_ptr);
	void operator delete[](void *a_ptr);
protected:
	int m_val;
};

void *demo::operator new(size_t a_size) throw(bad_alloc)
{
	cout << "new size=" << a_size << endl;
	void *p = NULL;
	p = malloc(a_size);
	if (!p) {
		bad_alloc a;
		throw (a);
	}
	return p;
}

void *demo::operator new[](size_t a_size) throw(bad_alloc)
{
	cout << "new[] size=" << a_size << endl;
	void *p = NULL;
	p = malloc(a_size);
	if (!p) {
		bad_alloc a;
		throw (a);
	}
	return p;
}

void demo::operator delete(void *a_ptr)
{
	cout << "delete ptr=" << hex << a_ptr << dec << endl;
	free(a_ptr);
}

void demo::operator delete[](void *a_ptr)
{
	cout << "delete[] ptr=" << hex << a_ptr << dec << endl;
	free(a_ptr);
}

int main(int argc, char **argv)
{
	demo *d1 = new demo(4);
	demo *d2 = new demo(7);
	delete d1;
	delete d2;
	demo *da = new demo[100];
	delete[] da;
	return 0;
}

