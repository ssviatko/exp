#include <stdio.h>
#include <stdint.h>

typedef struct {
	int16_t e;
	int m_s;
	__uint128_t m;
} manfp;

__uint128_t himask128 = ((__uint128_t)1 << 127);

void manfp_assign(manfp *a_manfp, __int128_t a_int)
{
	a_manfp->m = a_int;
	a_manfp->m_s = 0;
	a_manfp->e = 0x8000;
	if (a_int < 0) {
		a_manfp->m_s = -1;
		a_manfp->m *= -1;
	}
	int shift_cnt = 0;
	while ((a_manfp->m & himask128) == 0) {
		a_manfp->m <<= 1;
		shift_cnt++;
	}
	a_manfp->e = 0x8000 + (127 - shift_cnt);
}

void manfp_dump(manfp *a_manfp)
{
	printf("e:%04X m_s:%d ", a_manfp->e, a_manfp->m_s);
	for (int i = sizeof(a_manfp->m) - 1; i >= 0; --i) {
		unsigned char hexchar = ((char *)&a_manfp->m)[i];
		printf("%02X", hexchar);
	}
	printf("\n");
}

int main(int argc, char **argv)
{
	manfp num1;
	manfp_assign(&num1, 12345);
	manfp_dump(&num1);
	manfp_assign(&num1, 4);
	manfp_dump(&num1);
	manfp_assign(&num1, -7);
	manfp_dump(&num1);
	return 0;
}

