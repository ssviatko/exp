#include <stdio.h>
#include <stdlib.h>
#include <gmp.h>
#include <time.h>

#define DHBITS 2048
#define BYTEBUFF (DHBITS / 8)

int main(int argc, char **argv)
{
	// prepare random n-bit odd number
	srand(time(NULL));
	unsigned char l_bytes[BYTEBUFF];
	int i;
	for (i = 0; i < BYTEBUFF; ++i) {
		l_bytes[i] = rand() % 256;
	}
	l_bytes[0] |= 0x80; // make it between 2^n - 1 and 2^(n-1)
	l_bytes[BYTEBUFF - 1] |= 0x01; // make it odd

	printf("DH toolbox\n");
	mpz_t l_base;
	unsigned int l_exponent = 128;
	mpz_t l_max;
	mpz_t l_min;
	
	mpz_init_set_ui(l_base, 2);
	mpz_init(l_max);
	mpz_init(l_min);
	mpz_pow_ui(l_max, l_base, l_exponent);
	mpz_sub_ui(l_max, l_max, 1);
	l_exponent--;
	mpz_pow_ui(l_min, l_base, l_exponent);

	gmp_printf("min = %Zd (%Zx)\n", l_min, l_min);
	gmp_printf("max = %Zd (%Zx)\n", l_max, l_max);

	mpz_t l_randstart;
	mpz_init(l_randstart);
	gmp_randstate_t l_randstate;
	gmp_randinit_default(l_randstate);
	gmp_randseed_ui(l_randstate, time(NULL) * time(NULL));
	mpz_rrandomb(l_randstart, l_randstate, 256);
	gmp_printf("randstart = %Zd (%Zx)\n", l_randstart, l_randstart);
	gmp_randclear(l_randstate);

	printf("rand() random string: ");
	for (i = 0; i < BYTEBUFF; ++i) {
		printf("%02X", l_bytes[i]);
	}
	printf("\n");
	mpz_t l_randimport;
	mpz_init(l_randimport);
	mpz_import(l_randimport, BYTEBUFF, 1, sizeof(unsigned char), 0, 0, l_bytes);
	gmp_printf("randimport = %Zd (%Zx)\n", l_randimport, l_randimport);
	int l_pp = mpz_probab_prime_p(l_randimport, 50);
	printf("mpz_probab_prime_p returned %d.\n", l_pp);
	mpz_t l_nextprime;
	mpz_init(l_nextprime);
	mpz_nextprime(l_nextprime, l_randimport);
	gmp_printf("nextprime = %Zd (%Zx)\n", l_nextprime, l_nextprime);

	return 0;
}

