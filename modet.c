#include <stdio.h>
#include <sys/types.h>
#include <sys/fcntl.h>

int main(int argc, char **argv)
{
	mode_t mod = 0;

	mod |= S_IRUSR;

	printf("sizeof mod %ld mod=%04X\n", sizeof(mod), mod);
	return 0;
}

