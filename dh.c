#include <stdio.h>
#include <stdlib.h>
#include <gmp.h>
#include <time.h>
#include <getopt.h>

#define MAXBITS 8192
#define MAXBYTEBUFF (MAXBITS / 8)

unsigned char l_p[MAXBYTEBUFF];
unsigned char l_a[MAXBYTEBUFF];
unsigned char l_b[MAXBYTEBUFF];

struct option g_options[] = {
	{ "bits", required_argument, NULL, 'b' },
	{ NULL, 0, NULL, 0 }
};

int main(int argc, char **argv)
{
	unsigned int i;
	unsigned int l_bits = 512;
	int opt;
	srand(time(NULL));
	while ((opt = getopt_long(argc, argv, "b:", g_options, NULL)) != -1) {
		switch (opt) {
			case 'b':
				{
					l_bits = atoi(optarg);
				}
		}
	}
	if (l_bits > 8192) {
		fprintf(stderr, "dh: bit width too big for practical purposes.\n");
		exit(EXIT_FAILURE);
	}
	if (l_bits < 128) {
		fprintf(stderr, "dh: bit width too small for practical purposes.\n");
		exit(EXIT_FAILURE);
	}
	if ((l_bits % 64) != 0) {
		fprintf(stderr, "dh: bit width should be divisible by 64.\n");
		exit(EXIT_FAILURE);
	}

	int l_privbits = (l_bits / 8) + 96;

	printf("DH toolbox\n");
	printf("DH bit width: %d\n", l_bits);
	printf("Private bit width: %d\n", l_privbits);

	printf("Preparing %d-bit p value...\n", l_bits);
	// prepare random n-bit odd number for DH p factor
	for (i = 0; i < (l_bits / 8); ++i) {
		l_p[i] = rand() % 256;
	}
	l_p[0] |= 0x80; // make it between 2^n - 1 and 2^(n-1)
	l_p[(l_bits / 8) - 1] |= 0x01; // make it odd

	mpz_t l_p_import;
	mpz_init(l_p_import);
	mpz_import(l_p_import, (l_bits / 8), 1, sizeof(unsigned char), 0, 0, l_p);
	gmp_printf("p = %Zx\n", l_p_import);
	int l_pp = mpz_probab_prime_p(l_p_import, 50);
	printf("mpz_probab_prime_p returned %d.\n", l_pp);
	if (l_pp == 0) {
		printf("calling mpz_nextprime...\n");
		mpz_nextprime(l_p_import, l_p_import);
	}
	gmp_printf("p = %Zx\n", l_p_import);
	l_pp = mpz_probab_prime_p(l_p_import, 50);
	printf("mpz_probab_prime_p now returns %d.\n", l_pp);

	printf("Preparing %d-bit a value...\n", l_privbits);
	for (i = 0; i < (l_privbits / 8); ++i) {
		l_a[i] = rand() % 256;
	}
	mpz_t l_a_import;
	mpz_init(l_a_import);
	mpz_import(l_a_import, (l_privbits / 8), 1, sizeof(unsigned char), 0, 0, l_a);
	gmp_printf("a = %Zx\n", l_a_import);

	printf("Preparing %d-bit b value...\n", l_privbits);
	for (i = 0; i < (l_privbits / 8); ++i) {
		l_b[i] = rand() % 256;
	}
	mpz_t l_b_import;
	mpz_init(l_b_import);
	mpz_import(l_b_import, (l_privbits / 8), 1, sizeof(unsigned char), 0, 0, l_b);
	gmp_printf("b = %Zx\n", l_b_import);

	printf("Preparing g value...\n");
	mpz_t l_g;
	mpz_init(l_g);
	unsigned int l_g_rand = rand() % 16;
	// l_g_rand even/odd?
	if ((l_g_rand & 0x01) == 0)
		mpz_set_ui(l_g, 3);
	else
		mpz_set_ui(l_g, 5);
	gmp_printf("g = %Zd\n", l_g);

	/* at this point in the DH exchange, Alice would generate p and g and send them to Bob,
	 * so that both Alice and Bob have the same p and g. Then each Alice and Bob would
	 * generate a private value (a for Alice, b for Bob) and keep these secret. Then
	 * Alice computes A = g^a mod p, Bob computes B = g^b mod p, and Alice and Bob
	 * exchange A and B eith each other while keeping a and b secret. */

	mpz_t l_A;
	mpz_init(l_A);
	mpz_powm(l_A, l_g, l_a_import, l_p_import);
	gmp_printf("A = %Zx\n", l_A);

	mpz_t l_B;
	mpz_init(l_B);
	mpz_powm(l_B, l_g, l_b_import, l_p_import);
	gmp_printf("B = %Zx\n", l_B);

	/* Now Alice and Bob have each other's modexp computed values A and B, so Alice
	 * takes Bob's B and computes sa = B^a mod p. Bob takes Alice's A and computes
	 * sb = A^b mod p. When they are finished, sa and sb should match on either
	 * side of the transaction, and this value can be used to create a symmetric
	 * encryption key. */

	mpz_t l_sa;
	mpz_init(l_sa);
	mpz_powm(l_sa, l_B, l_a_import, l_p_import);
	gmp_printf("sa = %Zx\n", l_sa);

	mpz_t l_sb;
	mpz_init(l_sb);
	mpz_powm(l_sb, l_A, l_b_import, l_p_import);
	gmp_printf("sb = %Zx\n", l_sb);

	return 0;
}

