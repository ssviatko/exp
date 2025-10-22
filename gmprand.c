#include <gmp.h>
#include <time.h>

int main() {
    // Create and initialize the random state
    gmp_randstate_t state;
    gmp_randinit_default(state); // or gmp_randinit_mt(state);

    // Seed the random state with the current time
    gmp_randseed_ui(state, time(NULL));

    // Create and initialize an mpz_t variable to hold the random number
    mpz_t random_number;
    mpz_init(random_number);

    // Generate a random number with 50 bits
    mpz_urandomb(random_number, state, 50);

    // Print the random number
    gmp_printf("Random number: %Zd\n", random_number);

    // Clean up
    gmp_randclear(state);
    mpz_clear(random_number);

    return 0;
}
