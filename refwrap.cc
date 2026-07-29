#include <iostream>
#include <vector>
#include <functional>
#include <algorithm>

std::vector<int> bare = { 5, 2, 3, 8, 9, 1, 7};

int main(int argc, char **argv)
{
	std::vector<std::reference_wrapper<int>> barecopy;
	std::copy(bare.begin(), bare.end(), std::back_inserter(barecopy));
	std::sort(barecopy.begin(), barecopy.end(), [&](const auto &lhs, const auto &rhs) { return lhs < rhs; });
	for (auto i : bare) {
		std::cout << "bare: " << i << std::endl;
	}
	for (auto i : barecopy) {
		std::cout << "barecopy: " << i << std::endl;
	}
	return 0;
}

