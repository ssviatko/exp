#include <iostream>
#include <iomanip>
#include <cmath>

double mylog(double a_log)
{
	double ret;
	double sign = -1.0;
	double divider = a_log;
	double e = 2.71828182845904523536;
	int dividecount = 0;

	if (a_log <= 0.0)
		return INFINITY;

	while (divider > 1.7) {
		divider /= e;
		++dividecount;
	}

	while (divider < 0.3) {
		divider *= e;
		--dividecount;
	}

	double adjust = divider - 1.0;
	ret = adjust;
	for (int i = 2; i <= 84; ++i) {
		double term = adjust;
		for (int j = 1; j < i; ++j)
			term *= adjust;
		term /= (double)i;
		ret += term * sign;
//		std::cout << "term=" << term << " sign=" << sign << " ret=" << ret << " i=" << i << std::endl;
		sign *= -1.0;
	}

	if (dividecount) {
		ret += (double)dividecount;
	}

	return ret;
}

double mylog10(double a_log) {
	double ln10 = 2.302585092994045684;
	return (mylog(a_log) / ln10);
}

int main(int argc, char **argv)
{
	for (double tolog = 0.01; tolog <= 0.3; tolog+= 0.01) {
		std::cout << std::setprecision(15) << tolog << ": C++ ln = " << std::log(tolog) << " mylog ln = " << mylog(tolog) << std::endl;
	}
	for (double tolog = 0.1; tolog <= 3.0; tolog+= 0.1) {
		std::cout << std::setprecision(15) << tolog << ": C++ ln = " << std::log(tolog) << " mylog ln = " << mylog(tolog) << std::endl;
	}
	for (double tenlog = 0.0; tenlog < 30.0; tenlog += 0.5) {
		std::cout << std::setprecision(15) << tenlog << ": C++ log10 = " << std::log10(tenlog) << " log10 = " << mylog10(tenlog) << std::endl;
	}

	return 0;
}

