#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <utility>
#include <cmath>

const double e    = 2.71828182845904523536;
const double ln10 = 2.302585092994045684;

double myexp(double a_exp)
{
	double ret = 1.0;
	ret += a_exp; // 1 + x + x^2/2! + x^3/3!.. etc
	for (int i = 2; i <= 100; ++i) {
		double xsq = a_exp;
		for (int j = 2; j <= i; ++j)
			xsq *= a_exp; // x ^ i
		double fact = 1.0;
		for (int j = 2; j <= i; ++j)
			fact *= (double)j;
		ret += (xsq / fact);
	}
	return ret;
}

double mylog(double a_log)
{
	double ret;
	double sign = -1.0;
	double adjust = a_log;
	int dividecount = 0;

	if (a_log <= 0.0)
		return INFINITY;

	while (adjust > 1.7) {
		adjust /= e;
		++dividecount;
	}

	while (adjust < 0.3) {
		adjust *= e;
		--dividecount;
	}

	adjust -= 1.0; // ln(1 + x) Taylor series
	ret = adjust;
	for (int i = 2; i <= 84; ++i) {
		double term = adjust;
		for (int j = 1; j < i; ++j)
			term *= adjust;
		term /= (double)i;
		ret += term * sign;
		sign *= -1.0;
	}

	if (dividecount) {
		ret += (double)dividecount;
	}

	return ret;
}

double mylog10(double a_log) {
	return (mylog(a_log) / ln10);
}

int main(int argc, char **argv)
{
	const std::vector<std::pair<std::string, double> > ops = {
		{ "-0.1", -0.1 },
		{ "-0.4", -0.4 },
		{ "-1/2", -0.5 },
		{ "-1", -1.0 },
		{ "-2", -2.0 },
		{ "0.00000000028", 0.00000000028 },
		{ "0.0000002", 0.0000002 },
		{ "0.0001", 0.0001 },
		{ "0.1", 0.1 },
		{ "0.1", 0.1 },
		{ "0.4", 0.4 },
		{ "1/2", 0.5 },
		{ "1", 1.0 },
		{ "2", 2.0 },
		{ "e", 2.7818 },
		{ "3.4", 3.4 },
		{ "5.6", 5.6 },
		{ "7", 7.0 },
		{ "10", 10.0 },
		{ "15", 15.0 },
		{ "16", 16.0 },
		{ "17", 17.0 },
		{ "18", 18.0 },
		{ "22", 22.0 },
		{ "26", 26.0 },
		{ "32", 32.0 },
		{ "34", 34.0 },
		{ "36", 36.0 },
		{ "38", 38.0 }
	};
	
	for (auto i : ops) {
		std::cout << i.first << ": " << std::setprecision(15) << i.second << " log= " << std::log(i.second) << " mylog= " << mylog(i.second) << " exp= " << std::exp(i.second) << " myexp= " << myexp(i.second) << std::endl;
	}
	std::cout << "log 3.4^5.6   = " << std::setprecision(15) << std::exp(5.6 * std::log(3.4)) << std::endl;
	std::cout << "mylog 3.4^5.6 = " << std::setprecision(15) << std::exp(5.6 * mylog(3.4)) << std::endl;
	std::cout << "exp(2)   = " << std::setprecision(15) << std::exp(2.0) << std::endl;
	std::cout << "myexp(2) = " << std::setprecision(15) << myexp(2.0) << std::endl;
	std::cout << "myexp 3.4^5.6 = " << std::setprecision(15) << myexp(5.6 * mylog(3.4)) << std::endl;

	return 0;
}

