#include <iostream>

extern "C" float *doubler(float *floatarray);

float z[8] = {2.3, 5.8, 4.9, 9.3, 7.7, 6.456, 2.78, 0.987 };

int main(int argc, char **argv)
{
	float *res = doubler(z);
	for (int i = 0; i < 8; ++i)
		std::cout << "res" << i << " = " << res[i] << std::endl;

	return 0;
}
