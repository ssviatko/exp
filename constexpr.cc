#include <iostream>

constexpr double cdouble(double a_arg)
{
	return a_arg * 2;
}

int main(int argc, char **argv)
{
	for (int i = 3; i < 10; ++i) {
		std::cout << cdouble((double)i) << std::endl;
	}

	return 0;
}

