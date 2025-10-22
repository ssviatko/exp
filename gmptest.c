#include <stdio.h>
#include <gmp.h>
#include <time.h>

int main(int argc, char **argv)
{
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

	return 0;
}

