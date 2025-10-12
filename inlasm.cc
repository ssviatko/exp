#include <iostream>
#include <cstdint>

uint64_t adder(uint64_t a, uint64_t b, uint64_t c)
{
	uint64_t ret;

	asm (
		"movq %rdi, %rax\n"
		"addq %rsi, %rax\n"
		"addq %rcx, %rax\n"
		"movq %rax, %0\n"
		: "=r0"(ret)
	);
	return ret;
}

int main(int argc, char **argv)
{
	std::cout << "Result is " << adder(7, 6, 5) << std::endl;
	return 0;
}

