#include <stdio.h>
#include <string.h>
#include <stdint.h>

char hex128[17];

typedef union {
	__uint128_t val;
	unsigned char byte[16];
	struct {
		uint64_t lo;
		uint64_t hi;
	};
} union128_t;

char *pr128x(union128_t *a_int)
{
	int i;
	sprintf(hex128, "%016lX%016lX", a_int->hi, a_int->lo);
	return hex128;
}

int main(int argc, char **argv)
{
	union128_t a;
	int i;
	unsigned long long b = 0xfcdeabcd458c9012ULL;
	unsigned long long c = 0xecabdff8451c097aULL;
	a.val = (__uint128_t)b * (__uint128_t)c;
	printf("%016lX %016lX %s\n", b, c, pr128x(&a));
	a.val /= (__uint128_t)7654321;
	printf("%016lX %016lX %s\n", b, c, pr128x(&a));
	printf("sizeof __int128_t is %d\n", sizeof(a));
	return 0;
}

