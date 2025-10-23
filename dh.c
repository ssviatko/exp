#include <stdio.h>
#include <stdlib.h>
#include <gmp.h>
#include <time.h>

#define DHBITS 2048
#define DHBYTEBUFF (DHBITS / 8)
#define PRIVBITS 256
#define PRIVBYTEBUFF (PRIVBITS / 8)

unsigned char l_p[DHBYTEBUFF];
unsigned char l_a[PRIVBYTEBUFF];
unsigned char l_b[PRIVBYTEBUFF];

int main(int argc, char **argv)
{
	int i;
	srand(time(NULL));
	
	printf("DH toolbox\n");

	printf("Preparing %d-bit p value...\n", DHBITS);
	// prepare random n-bit odd number for DH p factor
	for (i = 0; i < DHBYTEBUFF; ++i) {
		l_p[i] = rand() % 256;
	}
	l_p[0] |= 0x80; // make it between 2^n - 1 and 2^(n-1)
	l_p[DHBYTEBUFF - 1] |= 0x01; // make it odd

	mpz_t l_p_import;
	mpz_init(l_p_import);
	mpz_import(l_p_import, DHBYTEBUFF, 1, sizeof(unsigned char), 0, 0, l_p);
	gmp_printf("p = %Zx\n", l_p_import);
	int l_pp = mpz_probab_prime_p(l_p_import, 50);
	printf("mpz_probab_prime_p returned %d.\n", l_pp);
	if (l_pp == 0) {
		printf("calling mpz_nextprime...\n");
		mpz_nextprime(l_p_import, l_p_import);
	}
	gmp_printf("p = %Zx\n", l_p_import);

	printf("Preparing %d-bit a value...\n", PRIVBITS);
	for (i = 0; i < PRIVBYTEBUFF; ++i) {
		l_a[i] = rand() % 256;
	}
	mpz_t l_a_import;
	mpz_init(l_a_import);
	mpz_import(l_a_import, PRIVBYTEBUFF, 1, sizeof(unsigned char), 0, 0, l_a);
	gmp_printf("a = %Zx\n", l_a_import);

	printf("Preparing %d-bit b value...\n", PRIVBITS);
	for (i = 0; i < PRIVBYTEBUFF; ++i) {
		l_b[i] = rand() % 256;
	}
	mpz_t l_b_import;
	mpz_init(l_b_import);
	mpz_import(l_b_import, PRIVBYTEBUFF, 1, sizeof(unsigned char), 0, 0, l_b);
	gmp_printf("b = %Zx\n", l_b_import);

	return 0;
}

