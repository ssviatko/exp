#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <gmp.h>
#include <time.h>
#include <getopt.h>
#include <pthread.h>

#define MAXBITS 16384
#define MAXBYTEBUFF (MAXBITS / 8)
#define MAXTHREADS 48

typedef struct {
	pthread_t thread;
	unsigned int id;
	unsigned char p[MAXBYTEBUFF];
	unsigned char q[MAXBYTEBUFF];
	unsigned char buff[MAXBYTEBUFF];
} thread_work_area;

thread_work_area twa[MAXTHREADS];

pthread_mutex_t g_bell_mtx;
int g_bell = 0;

struct option g_options[] = {
	{ "bits", required_argument, NULL, 'b' },
	{ "help", no_argument, NULL, '?' },
	{ "debug", no_argument, NULL, 'd' },
	{ "threads", required_argument, NULL, 't' },
	{ NULL, 0, NULL, 0 }
};

int g_debug = 0;
// note: for the keygen, g_bits now refers to the n/block size, not the p/q size
unsigned int g_bits = 256; // default bit width
unsigned int g_pqbits; // convenience value
pthread_mutex_t g_urandom_mtx;
int g_urandom_fd; // file descriptor for /dev/urandom
unsigned int g_threads = 1; // default number of threads

void print_hex(uint8_t *a_buffer, size_t a_len)
{
	int i;
	for (i = 0; i < a_len; ++i) {
		if (i % 32 == 0)
			printf("\n");
		printf("%02X ", a_buffer[i]);
	}
	printf("\n");
}

static void right_justify(size_t a_size, size_t a_offset, char *a_buff)
{
	// move a_size number of bytes over by a_offset in buffer a_buff
	int i;
	for (i = a_size - 1; i >= 0; --i) {
		a_buff[i + a_offset] = a_buff[i];
	}
	// zero out space we vacated in front
	for (i = 0; i < a_offset; ++i) {
		a_buff[i] = 0;
	}
}

void *gen_tf(void *arg)
{
	thread_work_area *a_twa;
	a_twa = arg;

	mpz_t l_p_import;
	mpz_init(l_p_import);
	mpz_t l_q_import;
	mpz_init(l_q_import);
	mpz_t l_p1;
	mpz_init(l_p1);
	mpz_t l_q1;
	mpz_init(l_q1);
	mpz_t l_n;
	mpz_init(l_n);
	mpz_t l_ct;
	mpz_init(l_ct);
	mpz_t l_e;
	mpz_init(l_e);
	mpz_t l_tmp;
	mpz_init(l_tmp);
	mpz_t l_d;
	mpz_init(l_d);
	mpz_t l_q2;
	mpz_init(l_q2);
	mpz_t l_counter;
	mpz_init(l_counter);
	int l_success = 0;
	unsigned int l_attempt = 1;
	int res;
	unsigned int i;

	while (l_success == 0) {
		pthread_mutex_lock(&g_bell_mtx);
		if (g_bell > 0) {
			// we didn't make it, so terminate
			pthread_mutex_unlock(&g_bell_mtx);
			pthread_exit(NULL);
			// if we made it here there was a problem
			return NULL;
		}
		pthread_mutex_unlock(&g_bell_mtx);

		if (g_debug) printf("tid %d: attempt %d to generate key...\n", a_twa->id, l_attempt++);
		else printf(".");

		// prepare random n-bit odd number for p factor
		pthread_mutex_lock(&g_urandom_mtx);
		res = read(g_urandom_fd, a_twa->p, (g_pqbits / 8));
		pthread_mutex_unlock(&g_urandom_mtx);
		if (res != (g_pqbits / 8)) {
			fprintf(stderr, "rsa-keygen: problems reading /dev/urandom: %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
		a_twa->p[0] |= 0x80; // make it between 2^n - 1 and 2^(n-1)
		a_twa->p[(g_pqbits / 8) - 1] |= 0x01; // make it odd

		mpz_import(l_p_import, (g_pqbits / 8), 1, sizeof(unsigned char), 0, 0, a_twa->p);
		int l_pp = mpz_probab_prime_p(l_p_import, 50);
		if (l_pp == 0) {
			mpz_nextprime(l_p_import, l_p_import);
		}

		if (g_debug) gmp_printf("tid %d: p       = %Zx\n", a_twa->id, l_p_import);

		l_pp = mpz_probab_prime_p(l_p_import, 50);

		// prepare random n-bit odd number for q factor
		pthread_mutex_lock(&g_urandom_mtx);
		res = read(g_urandom_fd, a_twa->q, (g_pqbits / 8));
		pthread_mutex_unlock(&g_urandom_mtx);
		if (res != (g_pqbits / 8)) {
			fprintf(stderr, "rsa-keygen: problems reading /dev/urandom: %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
		a_twa->q[0] &= 0x7f; // set up q to hopefully be < p/2
		a_twa->q[0] |= 0x40; // but not too little, please.. enforce first byte between 0x40 and 0x7f
		a_twa->q[(g_pqbits / 8) - 1] |= 0x01; // make it odd

		mpz_import(l_q_import, (g_pqbits / 8), 1, sizeof(unsigned char), 0, 0, a_twa->q);
		l_pp = mpz_probab_prime_p(l_q_import, 50);
		if (l_pp == 0) {
			mpz_nextprime(l_q_import, l_q_import);
		}

		if (g_debug) gmp_printf("tid %d: q       = %Zx\n",a_twa->id, l_q_import);

		l_pp = mpz_probab_prime_p(l_q_import, 50);

		// p and q should not be identical
		if (mpz_cmp(l_p_import, l_q_import) == 0) {
			if (g_debug) fprintf(stderr, "tid %d: error: p and q cannot be identical.", a_twa->id);
			continue;
		}

		// p should be > than 2q
		mpz_mul_ui(l_q2, l_q_import, 2);
		if (mpz_cmp(l_q2, l_p_import) >= 0) {
			if (g_debug) fprintf(stderr, "tid %d: error: p must be greater than 2q.\n", a_twa->id);
			continue;
		}

		// establish p-1 and q-1
		mpz_sub_ui(l_p1, l_p_import, 1);
		mpz_sub_ui(l_q1, l_q_import, 1);
		if (g_debug) gmp_printf("tid %d: (p - 1) = %Zx\n", a_twa->id, l_p1);
		if (g_debug) gmp_printf("tid %d: (q - 1) = %Zx\n", a_twa->id, l_q1);

		// p-1 and q-1 should not have small prime factors. Check both of them for all primes <100
		mpz_set_ui(l_counter, 2); // start with 3 as all even numbers are divisible by 2
		int l_bailout = 0;
		do {
			mpz_nextprime(l_counter, l_counter);
			//			gmp_printf("testing %Zd against P - 1...\n", l_counter);
			mpz_gcd(l_tmp, l_counter, l_p1);
			if (mpz_cmp(l_tmp, l_counter) == 0) {
				l_bailout = 1;
				break;
			}
		} while (mpz_cmp_ui(l_counter, 100) <= 0);
		if (l_bailout == 1) {
			if (g_debug) gmp_printf("tid %d: error: (p - 1) value has small prime factor of %Zd.\n", a_twa->id, l_counter);
			continue;
		}

		mpz_set_ui(l_counter, 2); // start with 3 as all even numbers are divisible by 2
		l_bailout = 0;
		do {
			mpz_nextprime(l_counter, l_counter);
			//			gmp_printf("testing %Zd against Q - 1...\n", l_counter);
			mpz_gcd(l_tmp, l_counter, l_q1);
			if (mpz_cmp(l_tmp, l_counter) == 0) {
				l_bailout = 1;
				break;
			}
		} while (mpz_cmp_ui(l_counter, 100) <= 0);
		if (l_bailout == 1) {
			if (g_debug) gmp_printf("tid %d: error: (q - 1) value has small prime factor of %Zd.\n", a_twa->id, l_counter);
			continue;
		}

		// prepare n = p * q
		mpz_mul(l_n, l_p_import, l_q_import);
		if (g_debug) gmp_printf("tid %d: n       = %Zx\n", a_twa->id, l_n);

		// prepare carmichael totient
		mpz_lcm(l_ct, l_p1, l_q1);
		if (g_debug) gmp_printf("tid %d: ct      = %Zx\n", a_twa->id, l_ct);

		// choose e, so that e is coprime with ct
		mpz_set_ui(l_e, 65536); // start at 65537 after nextprime is called
		do {
			mpz_nextprime(l_e, l_e);
			if (g_debug) gmp_printf("tid %d: testing e = %Zd...\n", a_twa->id, l_e);
			mpz_gcd(l_tmp, l_e, l_ct);
		} while (mpz_cmp_ui(l_tmp, 1) != 0);

		// choose d
		if (mpz_invert(l_d, l_e, l_ct) == 0) {
			if (g_debug) fprintf(stderr, "tid %d: invert failed!\n", a_twa->id);
			continue;
		} else {
			if (g_debug) gmp_printf("tid %d: d       = %Zx\n", a_twa->id, l_d);
		}
		l_success = 1; // made it this far, we generated a key pair!
	}

	pthread_mutex_lock(&g_bell_mtx);
	if (g_bell > 0) {
		// we didn't make it, so terminate
		pthread_mutex_unlock(&g_bell_mtx);
		pthread_exit(NULL);
		// if we made it here there was a problem
		return NULL;
	}
	g_bell = 1;
	pthread_mutex_unlock(&g_bell_mtx);
	printf("\ntid %d: Done.\n", a_twa->id);

	// export
	size_t l_written = 0;

	mpz_export(a_twa->buff, &l_written, 1, sizeof(unsigned char), 0, 0, l_n);
	if (l_written != (g_bits / 8)) {
		right_justify(l_written, (g_bits / 8) - l_written, (char *)a_twa->buff);
	}
	printf("modulus n (%d bits):", g_bits);
	print_hex(a_twa->buff, (g_bits / 8));

	mpz_export(a_twa->buff, &l_written, 1, sizeof(unsigned char), 0, 0, l_e);
	if (l_written != 4) { // save e as a 32 bit value, big endian
		right_justify(l_written, 4 - l_written, (char *)a_twa->buff);
	}
	printf("public exponent e:", g_bits);
	print_hex(a_twa->buff, 4);

	mpz_export(a_twa->buff, &l_written, 1, sizeof(unsigned char), 0, 0, l_d);
	if (l_written != (g_bits / 8)) {
		right_justify(l_written, (g_bits / 8) - l_written, (char *)a_twa->buff);
	}
	printf("private exponent d:", g_bits);
	print_hex(a_twa->buff, (g_bits / 8));

	mpz_export(a_twa->buff, &l_written, 1, sizeof(unsigned char), 0, 0, l_p_import);
	if (l_written != (g_pqbits / 8)) {
		right_justify(l_written, (g_pqbits / 8) - l_written, (char *)a_twa->buff);
	}
	printf("prime p:");
	print_hex(a_twa->buff, (g_pqbits / 8));

	mpz_export(a_twa->buff, &l_written, 1, sizeof(unsigned char), 0, 0, l_q_import);
	if (l_written != (g_pqbits / 8)) {
		right_justify(l_written, (g_pqbits / 8) - l_written, (char *)a_twa->buff);
	}
	printf("prime q:");
	print_hex(a_twa->buff, (g_pqbits / 8));

	// clean up
	mpz_clear(l_p_import);
	mpz_clear(l_q_import);
	mpz_clear(l_p1);
	mpz_clear(l_q1);
	mpz_clear(l_n);
	mpz_clear(l_ct);
	mpz_clear(l_e);
	mpz_clear(l_tmp);
	mpz_clear(l_d);
	mpz_clear(l_q2);
	mpz_clear(l_counter);

	// cancel everybody else
//	for (i = 0; i < g_threads; ++i) {
//		if (i != a_twa->id) {
//			pthread_cancel(twa[i].thread);
//		}
//	}

	if (g_bits > 2048)
		printf("\nPlease be patient while other threads terminate:\n(may take several seconds at large bit rates)\n");

	pthread_exit(NULL);
	return NULL;
}

int main(int argc, char **argv)
{
	unsigned int i;
	int res; // result variable for UNIX reads
	int opt;
	while ((opt = getopt_long(argc, argv, "db:?t:", g_options, NULL)) != -1) {
		switch (opt) {
			case 'd':
				{
					g_debug = 1;
				}
				break;
			case 't':
				{
					g_threads = atoi(optarg);
				}
				break;
			case 'b':
				{
					g_bits = atoi(optarg);
				}
				break;
			case '?':
				{
					printf("usage: rsa-keygen -b (--bits) <bit width>\n");
					printf("  RSA bit width must be between 256-8192 in 256 bit increments\n");
					printf("  default: %d bits\n", g_bits);
					exit(EXIT_SUCCESS);
				}
				break;
		}
	}
	if (g_bits > 16384) {
		fprintf(stderr, "rsa-keygen: bit width too big for practical purposes.\n");
		exit(EXIT_FAILURE);
	}
	if (g_bits < 256) {
		fprintf(stderr, "rsa-keygen: bit width too small for practical purposes.\n");
		exit(EXIT_FAILURE);
	}
	if ((g_bits % 256) != 0) {
		fprintf(stderr, "rsa-keygen: bit width should be divisible by 256.\n");
		exit(EXIT_FAILURE);
	}

	// police thread count
	if (g_threads < 1) {
		fprintf(stderr, "rsa-keygen: need to use at least 1 thread.\n");
		exit(EXIT_FAILURE);
	}
	if (g_threads > MAXTHREADS) {
		fprintf(stderr, "rsa-keygen: thread limit: %d.\n", MAXTHREADS);;
		exit(EXIT_FAILURE);
	}
	pthread_mutex_init(&g_bell_mtx, NULL);
	pthread_mutex_init(&g_urandom_mtx, NULL);

	g_pqbits = g_bits / 2;
	printf("RSA keygen\n");
	printf("RSA block bit width: %d\n", g_bits);
	if (g_debug > 0) {
		printf("debug mode enabled\n");
	}
	if (g_threads > 1)
		printf("enabling %d threads.\n", g_threads);

	// open urandom
	g_urandom_fd = open("/dev/urandom", O_RDONLY);
	if (g_urandom_fd < 0) {
		fprintf(stderr, "rsa: problems opening /dev/urandom: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	setbuf(stdout, NULL); // disable buffering so we can print our progress

	for (i = 0; i < g_threads; ++i) {
		twa[i].id = i;
		pthread_create(&twa[i].thread, NULL, gen_tf, &twa[i]);
	}

	// join
	for (i = 0; i < g_threads; ++i) {
//		printf("joining %d...\n", i);
		pthread_join(twa[i].thread, NULL);
	}

	pthread_mutex_destroy(&g_bell_mtx);
	pthread_mutex_destroy(&g_urandom_mtx);

	return 0;
}


