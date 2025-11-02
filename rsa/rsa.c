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
#include <arpa/inet.h>

#pragma pack(1)

#define MAXBITS 16384
#define MAXBYTEBUFF (MAXBITS / 8)

#define BUFFLEN 1024

uint32_t g_bits = 0;
uint8_t g_n[MAXBYTEBUFF];
int g_n_loaded = 0;
uint8_t g_e[sizeof(uint32_t)];
int g_e_loaded = 0;
uint8_t g_d[MAXBYTEBUFF];
int g_d_loaded = 0;

uint8_t g_buff[MAXBYTEBUFF]; // general buffer

const uint8_t KIHT_MODULUS = 1;
const uint8_t KIHT_PUBEXP = 2;
const uint8_t KIHT_PRIVEXP = 3;
const uint8_t KIHT_P = 4;
const uint8_t KIHT_Q = 5;

typedef struct {
    uint8_t type;
    uint32_t bit_width;
} key_item_header;

char g_infile[BUFFLEN];
int g_infile_specified = 0;
char g_outfile[BUFFLEN];
int g_outfile_specified = 0;
char g_keyfile[BUFFLEN];
int g_keyfile_specified = 0;

int g_debug = 0;

typedef enum {
    MODE_NONE,
    MODE_ENCRYPT,
    MODE_DECRYPT,
    MODE_SIGN,
    MODE_VERIFY,
    MODE_TEST
} operational_mode;

operational_mode g_mode = MODE_NONE;;

struct option g_options[] = {
    { "help", no_argument, NULL, '?' },
    { "debug", no_argument, NULL, 1001 },
    { "in", required_argument, NULL, 'i' },
    { "out", required_argument, NULL, 'o' },
    { "key", required_argument, NULL, 'k' },
    { "encrypt", no_argument, NULL, 'e' },
    { "decrypt", no_argument, NULL, 'd' },
    { "sign", no_argument, NULL, 's' },
    { "verify", no_argument, NULL, 'v' },
    { "test", no_argument, NULL, 't' },
    { NULL, 0, NULL, 0 }
};

void load_key()
{
    if (g_keyfile_specified == 0) {
        fprintf(stderr, "rsa: this operation requires that you specify a key file.\n");
        exit(EXIT_FAILURE);
    }

    // load keyfile up and populate n, e, d, etc
    int key_fd;
    key_fd = open(g_keyfile, O_RDONLY);
    if (key_fd < 0) {
        fprintf(stderr, "rsa: unable to open key file: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    int l_eof = 0;
    int res;
    do {
        key_item_header l_kih;
        res = read(key_fd, &l_kih, sizeof(l_kih));
        if (res == 0) {
            l_eof = 1;
            continue;
        }
        if (res != sizeof(l_kih)) {
            fprintf(stderr, "rsa: problems reading key file: unexpected end of file.\n");
            exit(EXIT_FAILURE);
        }

        if (l_kih.type == KIHT_MODULUS) {
            // if we read the modulus, set the over bit width
            g_bits = ntohl(l_kih.bit_width);
            printf("rsa: selected %d bit width.\n", g_bits);
            res = read(key_fd, g_n, (g_bits / 8));
            if (res != (g_bits / 8)) {
                fprintf(stderr, "rsa: problems reading key file: can't read modulus.\n");
                exit(EXIT_FAILURE);
            }
            g_n_loaded = 1;
        } else if (l_kih.type == KIHT_PUBEXP) {
            res = read(key_fd, g_e, sizeof(uint32_t));
            if (res != sizeof(uint32_t)) {
                fprintf(stderr, "rsa: problems reading key file: can't read public exponent.\n");
                exit(EXIT_FAILURE);
            }
            g_e_loaded = 1;
        } else if (l_kih.type == KIHT_PRIVEXP) {
            res = read(key_fd, g_d, (ntohl(l_kih.bit_width) / 8));
            if (res != (ntohl(l_kih.bit_width) / 8)) {
                fprintf(stderr, "rsa: problems reading key file: can't read private exponent.\n");
                exit(EXIT_FAILURE);
            }
            g_d_loaded = 1;
        } else {
            // that's all we care about for now, just throw away everything else
            res = read(key_fd, g_buff, (ntohl(l_kih.bit_width) / 8));
            if (res != (ntohl(l_kih.bit_width) / 8)) {
                fprintf(stderr, "rsa: problems reading key file: can't read unspecified field.\n");
                exit(EXIT_FAILURE);
            }
        }
    } while (l_eof == 0);
}

int main(int argc, char **argv)
{
    unsigned int i;
    int res; // result variable for UNIX reads
    int opt;
    while ((opt = getopt_long(argc, argv, "i:o:k:edsv?t", g_options, NULL)) != -1) {
        switch (opt) {
            case 1001:
            {
                g_debug = 1;
            }
            break;
            case 'i':
            {
                strcpy(g_infile, optarg);
                g_infile_specified = 1;
            }
            break;
            case 'o':
            {
                strcpy(g_outfile, optarg);
                g_outfile_specified = 1;
            }
            break;
            case 'k':
            {
                strcpy(g_keyfile, optarg);
                g_keyfile_specified = 1;
            }
            break;
            case 'e':
            {
                if (g_mode != MODE_NONE) {
                    fprintf(stderr, "rsa: please select only one operational mode.\n");
                    exit(EXIT_FAILURE);
                }
                g_mode = MODE_ENCRYPT;
            }
            break;
            case 'd':
            {
                if (g_mode != MODE_NONE) {
                    fprintf(stderr, "rsa: please select only one operational mode.\n");
                    exit(EXIT_FAILURE);
                }
                g_mode = MODE_DECRYPT;
            }
            break;
            case 's':
            {
                if (g_mode != MODE_NONE) {
                    fprintf(stderr, "rsa: please select only one operational mode.\n");
                    exit(EXIT_FAILURE);
                }
                g_mode = MODE_SIGN;
            }
            break;
            case 'v':
            {
                if (g_mode != MODE_NONE) {
                    fprintf(stderr, "rsa: please select only one operational mode.\n");
                    exit(EXIT_FAILURE);
                }
                g_mode = MODE_VERIFY;
            }
            break;
            case 't':
            {
                if (g_mode != MODE_NONE) {
                    fprintf(stderr, "rsa: please select only one operational mode.\n");
                    exit(EXIT_FAILURE);
                }
                g_mode = MODE_TEST;
            }
            break;
            case '?':
            {
                printf("usage: rsa <options>\n");
                printf("  -i (--in) <name> input file\n");
                printf("  -o (--out) <name> output file\n");
                printf("  -k (--key) <name> full name of key file to use\n");
                printf("     (--debug) use debug mode\n");
                printf("  -? (--help) this screen\n");
                printf("operational modes (select only one)\n");
                printf("  -e (--encrypt) encrypt mode\n");
                printf("       encrypts in->out with public key\n");
                printf("  -d (--decrypt) decrypt mode\n");
                printf("       decrypts in->out with private key\n");
                printf("  -s (--sign) sign mode (SHA2-256)\n");
                printf("       encrypts in->out with private key and signs contents with private key\n");
                printf("  -v (--verify) verify mode\n");
                printf("       decrypts in->out with public key and verifies contents against signature\n");
                printf("  -t (--test) test mode\n");
                exit(EXIT_SUCCESS);
            }
            break;
        }
    }

    if (g_debug > 0)
        printf("rsa: debug mode enabled.\n");

    if (g_infile_specified > 0) {
        printf("rsa: input file : %s\n", g_infile);
    }
    if (g_outfile_specified > 0) {
        printf("rsa: output file: %s\n", g_outfile);
    }
    if (g_keyfile_specified > 0) {
        printf("rsa: key file   : %s\n", g_keyfile);
    }

    switch (g_mode) {
        case MODE_NONE:
        {
            fprintf(stderr, "rsa: you must select one operational mode.\n");
	    fprintf(stderr, "rsa: use -? or --help for usage info.\n");
            exit(EXIT_FAILURE);
        }
        break;
        case MODE_ENCRYPT:
        {
            printf("rsa: selected encryption mode.\n");
            load_key();
            if (g_n_loaded == 0) {
                fprintf(stderr, "rsa: this function requires the key file to contain a modulus.\n");
                exit(EXIT_FAILURE);
            }
            if (g_e_loaded == 0) {
                fprintf(stderr, "rsa: this function requires the key file to contain a public exponent.\n");
                exit(EXIT_FAILURE);
            }
            if (g_infile_specified == 0) {
                fprintf(stderr, "rsa: this function requires that you specify an input file.\n");
                exit(EXIT_FAILURE);
            }
            if (g_outfile_specified == 0) {
                fprintf(stderr, "rsa: this function requires that you specify an output file.\n");
                exit(EXIT_FAILURE);
            }
        }
        break;
        case MODE_DECRYPT:
        {
            printf("rsa: selected decryption mode.\n");
            load_key();
            if (g_n_loaded == 0) {
                fprintf(stderr, "rsa: this function requires the key file to contain a modulus.\n");
                exit(EXIT_FAILURE);
            }
            if (g_d_loaded == 0) {
                fprintf(stderr, "rsa: this function requires the key file to contain a private exponent.\n");
                exit(EXIT_FAILURE);
            }
            if (g_infile_specified == 0) {
                fprintf(stderr, "rsa: this function requires that you specify an input file.\n");
                exit(EXIT_FAILURE);
            }
            if (g_outfile_specified == 0) {
                fprintf(stderr, "rsa: this function requires that you specify an output file.\n");
                exit(EXIT_FAILURE);
            }
        }
        break;
        case MODE_SIGN:
        {
            printf("rsa: selected sign mode.\n");
            load_key();
            if (g_n_loaded == 0) {
                fprintf(stderr, "rsa: this function requires the key file to contain a modulus.\n");
                exit(EXIT_FAILURE);
            }
            if (g_d_loaded == 0) {
                fprintf(stderr, "rsa: this function requires the key file to contain a private exponent.\n");
                exit(EXIT_FAILURE);
            }
            if (g_infile_specified == 0) {
                fprintf(stderr, "rsa: this function requires that you specify an input file.\n");
                exit(EXIT_FAILURE);
            }
            if (g_outfile_specified == 0) {
                fprintf(stderr, "rsa: this function requires that you specify an output file.\n");
                exit(EXIT_FAILURE);
            }
        }
        break;
        case MODE_VERIFY:
        {
            printf("rsa: selected verify mode.\n");
            load_key();
            if (g_n_loaded == 0) {
                fprintf(stderr, "rsa: this function requires the key file to contain a modulus.\n");
                exit(EXIT_FAILURE);
            }
            if (g_e_loaded == 0) {
                fprintf(stderr, "rsa: this function requires the key file to contain a public exponent.\n");
                exit(EXIT_FAILURE);
            }
            if (g_infile_specified == 0) {
                fprintf(stderr, "rsa: this function requires that you specify an input file.\n");
                exit(EXIT_FAILURE);
            }
            if (g_outfile_specified == 0) {
                fprintf(stderr, "rsa: this function requires that you specify an output file.\n");
                exit(EXIT_FAILURE);
            }
        }
        break;
        case MODE_TEST:
        {
            printf("rsa: selected test mode.\n");
        }
        break;
        default:
        {
            printf("I don't know what to do!\n");
        }
        break;
    }

    return 0;
}
