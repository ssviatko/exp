#include <iostream>
#include <string>
#include <tuple>
#include <format>
#include <chrono>
#include <source_location>
#include <stdio.h>

template <typename T>
void printit(const T& a_val)
{
	std::cout << "boundary: " << a_val << std::endl;
}

template <typename U, typename... T>
void printit(const U& a_val, const T&... a_other_vals)
{
	std::cout << "got value: " << a_val << std::endl;
	printit(a_other_vals...);
}

template <typename... T>
void printf_wrapper(std::tuple<T...> const& argtuple, const std::source_location loc = std::source_location::current())
{
	//std::source_location loc = std::source_location::current();
	std::cout << "file: " << loc.file_name() << " line: " << loc.line() << " fn: " << loc.function_name() << " ";
	std::apply([](auto&&... args) { ((std::cout << " " << args), ...); }, argtuple);
	std::cout << std::endl;
}

void log_alternative(std::string&& msg, const std::source_location loc = std::source_location::current())
{
	std::chrono::high_resolution_clock::time_point l_now = std::chrono::high_resolution_clock::now();
	std::cout << "[" << std::format("{0:%F}T{0:%T}", l_now) << "] [" << loc.file_name() << ":" << loc.line() << "/" << loc.function_name() << "] " << msg << std::endl;
}

template <typename T>
std::string comma_list(const T& a_last)
{
	return a_last;
}

template <typename U, typename... T>
std::string comma_list(const U& a_first, const T&... a_other)
{
	return a_first + std::string(", ") + comma_list(a_other...);
}

int main(int argc, char **argv)
{
	int i = 7, j = 8, k = 9;
	float f = 3.14, g = 6.023, h = 2.7818;

	printit(i, f, j, g);
	printit(2, 3, 4.1);

	printf_wrapper(std::tuple(i, j));
	printf_wrapper(std::tuple(k, f, g));
	log_alternative(std::format("our values are {}, {}, {}, and {}.", h, f, k, i));

	std::cout << "Happy comma_list: " << comma_list("Apple", "Orange", "Lemon", "Lime", "Blueberry", "Grape") << std::endl;

	return 0;
}

