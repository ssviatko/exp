#include <iostream>
#include <iomanip>

#include <cmath>
#include <cstdint>

double mysin(double a_val)
{
	double adjust = a_val;
	const double pi = 3.1415926535897932384;
	const double pi2 = pi * 2.0;

	while (adjust > pi) {
		adjust -= pi2;
	}

	if (adjust < -pi) {
		adjust += pi2;
	}

	double ret = adjust;

	double sign = -1.0;

	auto factorial = [](unsigned long long f) -> unsigned long long {
		unsigned long long lambda_ret = 1;
		for (unsigned long long i = 1; i <= f; ++i)
			lambda_ret *= i;
		return lambda_ret;
	};

	for (unsigned long long i = 3; i <= 24; i += 2) {
		double term = adjust;
		for (unsigned long long j = 1; j < i; ++j)
			term *= adjust;
		term /= factorial(i);
		ret += term * sign;
		sign *= -1.0;
	}

	return ret;
}

double myasin(double a_val)
{
	// asin(-x = -asin(x)
	double sign = 1.0;
	if (a_val < 0.0)
		sign = -1.0;
	double work = std::abs(a_val); // first term ...

	// conventional taylor method
	double res = a_val;
	double fact = a_val; // ...equals the first factor

// this is only accurate to 8 decimal places, so we abandoned it
//	if (work > 0.999) {
//		// for values close to 1, use the Abramowitz/Stegun method:
//		const double pi = 3.1415926535897932384;
//		const double a0 = 1.5707963050;
//		const double a1 = -0.2145988016;
//		const double a2 = 0.0889789874;
//		const double a3 = -0.0501743046;
//		const double a4 = 0.0308918810;
//		const double a5 = -0.0170881256;
//		const double a6 = 0.0066700901;
//		const double a7 = -0.0012624911;
//		res = pi/2.0 - sqrt(1 - work) * (a0 + a1*work + a2*pow(work, 2) + a3*pow(work, 3) + a4*pow(work, 4) + a5*pow(work, 5) + a6*pow(work, 6) + a7*pow(work, 7));
//		printf(" ref %1.10f abr/ste  ", asin(a_val));
//		return res * sign;
//	}
//
	// use the identity asin(x) = pi/2 - asin(sqrt(1-x^2)) if a_val is > a specified cutoff
	if (work > 0.9) {
		const double pi = 3.1415926535897932384;
		work = pow(work, 2); // square it
		work = 1.0 - work; // subtract it from one
		work = std::sqrt(work); // take square root of it
		work = myasin(work); // recursively call ourselves to get the arcsine of this new number
		work = (pi / 2.0) - work;
		printf(" ref %1.10f adj ", asin(a_val));
		return work * sign;
	}

	int n;
	for (n = 0; n < 50000; n++) {
		double old = res;
		// calculate term(n + 1) as per the formulas above
		fact *= (2*n + 1) * (2*n + 2);
		fact /= 4.0 * (n + 1)*(n + 1) * (2*n + 3);
		fact *= a_val * a_val * (2*n + 1);

		res += fact;
		if (res == old) break;

//		printf("[%d] %f\n", n, res);
	}

	printf(" ref %1.10f terms=%d  ", asin(a_val), n);
	return res;
}

int main(int argc, char **argv)
{
        const double pi = 3.1415926535897932384;

	for (double d = -1.6; d <= 1.6; d += 0.01) {
		double ds = mysin(d);
		std::cout << std::setprecision(15) << std::fixed << std::setw(12) << d << " C++ sin=" << std::sin(d) << " mysin=" << ds << " myasin=" << myasin(ds) << std::endl;
	}
	// common sines
	std::cout << std::setprecision(15) << std::fixed << std::setw(12) << (pi / 2.0) << "  pi/2 = " << std::sin(pi / 2.0) << " mysin=" << mysin(pi / 2.0) << std::endl;
	std::cout << std::setprecision(15) << std::fixed << std::setw(12) << (pi / 3.0) << "  pi/3 = " << std::sin(pi / 3.0) << " mysin=" << mysin(pi / 3.0) << std::endl;
	std::cout << std::setprecision(15) << std::fixed << std::setw(12) << (pi / 4.0) << "  pi/4 = " << std::sin(pi / 4.0) << " mysin=" << mysin(pi / 4.0) << std::endl;
	std::cout << std::setprecision(15) << std::fixed << std::setw(12) << (pi / 6.0) << "  pi/6 = " << std::sin(pi / 6.0) << " mysin=" << mysin(pi / 6.0) << std::endl;

	return 0;
}
