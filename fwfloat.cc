/* in gcc 14, compile with:
 * g++ fwfloat.cc -o fwfloat -std=c++23 -Wl,-rpath=/usr/local/lib64
*/

#include <iostream>
#include <stdfloat>
#include <format>

int main(int argc, char **argv)
{
	std::float128_t ff = 3.141592653589701234567890123456789f128;
//	std::cout << "Our number is " << ff << std::endl;
	std::cout << std::format("Our number is {}.", ff) << std::endl;
	std::cout << "our float128 is " << sizeof(ff) << " bytes in size." << std::endl;
	return 0;
}

