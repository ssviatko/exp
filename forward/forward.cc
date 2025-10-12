#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <typeinfo>
#include <cstdint>

// our move/forward aware class

template <typename T>
class moveaware {
public:
	moveaware();
	moveaware(T a_T);
	moveaware(const moveaware<T>& a_other_ma);
	moveaware<T>& operator=(const moveaware<T>& a_other_ma);
	moveaware(moveaware<T>&& a_other_ma);
	moveaware<T>& operator=(moveaware<T>&& a_other_ma);
	virtual ~moveaware();
	void display();
	template <typename U> friend std::ostream& operator<<(std::ostream& os, const moveaware<U>& ma);
protected:
	T m_T;
};

template<typename U>
std::ostream& operator<<(std::ostream& os, const moveaware<U>& ma)
{
	return os << "(" << std::hex << std::setfill('0') << std::setw(16) << (std::uint64_t)&ma << "- " << typeid(U).name() << " = " << ma.m_T << ")";
}

// constructors

template<typename T>
moveaware<T>::moveaware()
{
	std::cout << std::hex << std::setfill('0') << std::setw(16) << (std::uint64_t)this << " moveaware type=" << typeid(T).name() << " default construct";
	std::cout << std::endl;
}

template<typename T>
moveaware<T>::moveaware(T a_T)
: m_T(a_T)
{
	std::cout << std::hex << std::setfill('0') << std::setw(16) << (std::uint64_t)this << " moveaware type=" << typeid(T).name() << " value=" << m_T << " construct";
	std::cout << std::endl;
}

template<typename T>
moveaware<T>::moveaware(const moveaware<T>& a_other_ma)
{
	m_T = a_other_ma.m_T;
	std::cout << std::hex << std::setfill('0') << std::setw(16) << (std::uint64_t)this << " moveaware type=" << typeid(T).name();
	std::cout << " copy from " << (std::uint64_t)&a_other_ma << " value=" << a_other_ma.m_T;
	std::cout << std::endl;
}

template<typename T>
moveaware<T>::moveaware(moveaware<T>&& a_other_ma)
{
	m_T = std::move(a_other_ma.m_T);
	std::cout << std::hex << std::setfill('0') << std::setw(16) << (std::uint64_t)this << " moveaware type=" << typeid(T).name();
	std::cout << " move from " << (std::uint64_t)&a_other_ma << " value=" << m_T;
	std::cout << std::endl;
}

// assignment operators

template<typename T>
moveaware<T>& moveaware<T>::operator=(const moveaware<T>& a_other_ma)
{
	m_T = a_other_ma.m_T;
	std::cout << std::hex << std::setfill('0') << std::setw(16) << (std::uint64_t)this << " moveaware type=" << typeid(T).name();
	std::cout << " copy assign from " << (std::uint64_t)&a_other_ma << " value=" << a_other_ma.m_T;
	std::cout << std::endl;
	return *this;
}

template<typename T>
moveaware<T>& moveaware<T>::operator=(moveaware<T>&& a_other_ma)
{
	m_T = std::move(a_other_ma.m_T);
	std::cout << std::hex << std::setfill('0') << std::setw(16) << (std::uint64_t)this << " moveaware type=" << typeid(T).name();
	std::cout << " move assign from " << (std::uint64_t)&a_other_ma << " value=" << m_T;
	std::cout << std::endl;
	return *this;
}

// destructor

template<typename T>
moveaware<T>::~moveaware()
{
	std::cout << std::hex << std::setfill('0') << std::setw(16) << (std::uint64_t)this << " moveaware type=" << typeid(T).name() << " value=" << m_T << " destruct";
	std::cout << std::endl;
}

// utility/support

template<typename T>
void moveaware<T>::display()
{
	std::cout << std::hex << std::setfill('0') << std::setw(16) << (std::uint64_t)this << " moveaware type=" << typeid(T).name() << " value=" << m_T;
	std::cout << std::endl;
}

/* owned pointer */

template<typename T>
class owned_ptr {
public:
	owned_ptr(T *a_ptr);
	~owned_ptr();
	T* operator->() const;
	T& operator*() const;
protected:
	T* m_ptr;
};

template<typename T>
owned_ptr<T>::owned_ptr(T *a_ptr)
: m_ptr(a_ptr)
{
	std::cout << std::hex << std::setfill('0') << std::setw(16) << (std::uint64_t)this << " owned_ptr type=" << typeid(T).name() << " ptr=" << std::setw(16) << (std::uint64_t)m_ptr << " construct";
	std::cout << std::endl;
}

template<typename T>
owned_ptr<T>::~owned_ptr()
{
	if (m_ptr != nullptr) {
		std::cout << std::hex << std::setfill('0') << std::setw(16) << (std::uint64_t)this << " owned_ptr type=" << typeid(T).name() << " ptr=" << std::setw(16) << (std::uint64_t)m_ptr << " destruct";
		std::cout << std::endl;
		delete m_ptr;
	}
}

template<typename T>
T* owned_ptr<T>::operator->() const
{
	return m_ptr;
}

template<typename T>
T& owned_ptr<T>::operator*() const
{
	return *m_ptr;
}

template<typename T, typename... Args>
owned_ptr<T> make_owned(Args&&... args)
{
	return owned_ptr<T>(new T(std::forward<Args>(args)...));
}

/* Main test program */

void test_owner()
{
	owned_ptr<moveaware<int> > oa = make_owned<moveaware<int> >(997);
	std::cout << "oa object is " << *oa << std::endl;
	owned_ptr<std::vector<int> > ov = make_owned<std::vector<int> >();
	ov->push_back(5);
	ov->push_back(7);
	std::cout << "vector contains " << ov->size() << " items." << std::endl;
}

int main(int argc, char **argv)
{
	std::cout << "create a" << std::endl;
	moveaware<std::string> a("Hello there");
	a.display();
	std::cout << a << std::endl;
	std::cout << "create b and copy from a" << std::endl;
	moveaware<std::string> b(a);
	b.display();
	std::cout << "a = b" << std::endl;
	a = b;
	std::cout << "create c and cannibalize from a" << std::endl;
	moveaware<std::string> c(std::move(a));
	std::cout << "create d and move assign from b" << std::endl;
	moveaware<std::string> d("nothing");
	d = std::move(b);

	std::cout << "moveable containing moveable - create inside instance" << std::endl;
	moveaware<int> f(7);
	std::cout << "moveable containing moveable - create outside instance" << std::endl;
	moveaware<moveaware<int> > e(f);
	std::cout << e << std::endl;
	std::cout << "triple stack" << std::endl;
	moveaware<moveaware<moveaware<int> > > ee(std::move(e));
	std::cout << ee << std::endl;

//	std::cout << "moveable containing moveable string- create inside instance" << std::endl;
//	moveaware<std::string> g("seven");
//	std::cout << "moveable containing moveable string - create outside instance" << std::endl;
//	moveaware<moveaware<std::string> > h(std::move(g));
//	std::cout << h << std::endl;

	std::cout << "test owned pointer" << std::endl;
	test_owner();
	
	std::cout << "end program..." << std::endl;
	return 0;
}

