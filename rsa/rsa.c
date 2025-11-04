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
#include <sys/stat.h>

#pragma pack(1)

#define MAXBITS 16384
#define MAXBYTEBUFF (MAXBITS / 8)

#define BUFFLEN 1024
#define PADDING 12 // amount of random padding per block

uint8_t g_n[MAXBYTEBUFF];
int g_n_loaded = 0;
uint8_t g_e[sizeof(uint32_t)];
int g_e_loaded = 0;
uint8_t g_d[MAXBYTEBUFF];
int g_d_loaded = 0;

uint8_t g_buff[MAXBYTEBUFF]; // general buffer
uint8_t g_buff2[MAXBYTEBUFF]; // auziliary buffer

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
int g_infile_fd;
off_t g_infile_length;
uint32_t g_infile_crc; // calculated during encrypt and sign operations

char g_outfile[BUFFLEN];
int g_outfile_specified = 0;
int g_outfile_fd;
int g_outfile_overwrite = 0;
uint32_t g_outfile_crc; // calculated during decrypt and verify

char g_keyfile[BUFFLEN];
int g_keyfile_specified = 0;
uint32_t g_bits = 0;

int g_urandom_fd;

// block related
uint32_t g_block_size;
int g_infile_block_multiple = 0; // is infile a multiple of the specified block size?
uint32_t g_block_capacity; // block size minus padding bytes, based on key size
uint32_t g_1stblock_capacity; // first block capacity minus fileinfo header

typedef struct {
    uint32_t size;
    uint32_t size_xor;
    uint32_t crc;
    uint32_t crc_xor;
} fileinfo_header;

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
    { "overwrite", no_argument, NULL, 'w' },
    { NULL, 0, NULL, 0 }
};

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
            printf("rsa: selected %d bit key.\n", g_bits);
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

void prepare_outfile()
{
    int res;

    // find out if outfile exists
    struct stat l_outfile_stat;
    res = stat(g_outfile, &l_outfile_stat);
    if (res == 0) {
        // successfully stat-ted the file. do we want to overwrite it?
        if (g_outfile_overwrite == 0) {
            fprintf(stderr, "rsa: output file already exists (use -w or --overwrite to write to it anyway)\n");
            exit(EXIT_FAILURE);
        } else {
            printf("rsa: overwriting existing output file %s\n", g_outfile);
        }
    } else if ((res < 0) && (errno == ENOENT)) {
        // this is what we want
    } else {
        // some other error from stat!
        fprintf(stderr, "rsa: unable to stat output file to check its existence: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // open the output file
    if (g_debug) printf("prepare_outfile: opening and truncating output file\n");
    g_outfile_fd = open(g_outfile, O_RDWR | O_TRUNC | O_CREAT, (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH));
    if (g_outfile_fd < 0) {
        fprintf(stderr, "rsa: error opening output file: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

void prepare_infile()
{
    int res;

    // find out infile length
    struct stat l_infile_stat;
    res = stat(g_infile, &l_infile_stat);
    if (res < 0) {
        fprintf(stderr, "rsa: error calling stat on input file: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    g_infile_length = l_infile_stat.st_size;
    if (g_debug) printf("prepare_infile: input file length: %d\n", g_infile_length);
    g_block_size = (g_bits / 8);
    if (g_debug) printf("prepare_infile: block size: %d bytes\n", g_block_size);
    uint32_t l_sizemod = (g_infile_length % g_block_size);
    if (l_sizemod > 0) g_infile_block_multiple = 1;
    if (g_debug) printf("prepare_infile: input file block multiple: %s\n", (g_infile_block_multiple ? "NO" : "YES"));
    g_block_capacity = g_block_size - PADDING;
    if (g_debug) printf("prepare_infile: block capacity: %d bytes\n", g_block_capacity);
    g_1stblock_capacity = g_block_capacity - sizeof(fileinfo_header);
    if (g_debug) printf("prepare_infile: first block capacity: %d bytes\n", g_1stblock_capacity);

    // open infile
    g_infile_fd = open(g_infile, O_RDONLY);
    if (g_infile_fd < 0) {
        fprintf(stderr, "rsa: problems opening input file: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

uint32_t g_crc32_tab[] = {
    0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
    0xe963a535, 0x9e6495a3,	0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
    0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
    0xf3b97148, 0x84be41de,	0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
    0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec,	0x14015c4f, 0x63066cd9,
    0xfa0f3d63, 0x8d080df5,	0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
    0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,	0x35b5a8fa, 0x42b2986c,
    0xdbbbc9d6, 0xacbcf940,	0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
    0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
    0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
    0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,	0x76dc4190, 0x01db7106,
    0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
    0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
    0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
    0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
    0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
    0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
    0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
    0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
    0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
    0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
    0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
    0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
    0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
    0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
    0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
    0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
    0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
    0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
    0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
    0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
    0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
    0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
    0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
    0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
    0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
    0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
    0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
    0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
    0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
    0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
    0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
    0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

uint32_t get_file_crc(int a_fd)
{
    uint32_t l_crc = 0;
    l_crc = l_crc ^ ~0U;

    uint8_t l_buff[4096]; // buffer our reads so this doesn't take forever'
    int res;
    int i;

    do {
        res = read(a_fd, l_buff, 4096);
        if (res == 0)
            continue; // got our EOF
        if (res < 0) {
            fprintf(stderr, "rsa: unable to compute CRC: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        // compute CRC for res number of bytes
        for (i = 0; i < res; ++i) {
            l_crc = g_crc32_tab[(l_crc ^ l_buff[i]) & 0xFF] ^ (l_crc >> 8);
        }
    } while (res != 0);

    return l_crc ^ ~0U;
}

void get_infile_crc()
{
    int res;
    g_infile_crc = get_file_crc(g_infile_fd);
    // rewind g_infile_fd
    res = lseek(g_infile_fd, 0, SEEK_SET);
    if (res < 0) {
        fprintf(stderr, "res: unable to rewind input file after computing CRC: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (g_debug > 0) printf("get_infile_crc: CRC is %08X\n", g_infile_crc);
}

void get_outfile_crc()
{
    int res;
    // rewind g_outfile_fd after writing data
    res = lseek(g_outfile_fd, 0, SEEK_SET);
    if (res < 0) {
        fprintf(stderr, "res: unable to rewind input file after computing CRC: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    g_outfile_crc = get_file_crc(g_outfile_fd);
    if (g_debug > 0) printf("get_outfile_crc: CRC is %08X\n", g_outfile_crc);
}

void get_random(uint8_t *a_buffer, size_t a_len)
{
    int res;
    res = read(g_urandom_fd, a_buffer, a_len);
    if (res != a_len) {
        fprintf(stderr, "rsa: problems reading /dev/urandom: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

void do_encrypt()
{
    int lastblock = 0; // flag to indicate we have run out of data, this is the last block
    int l_block_ctr = 0;
    int res;

    // prepare first block
    l_block_ctr++;
    get_random(g_buff, g_block_size);
    // PKCS#1 style padding of first two bytes
    g_buff[0] = 0;
    // prepare fileinfo header
    fileinfo_header l_fih;
    l_fih.size = htonl(g_infile_length);
    l_fih.size_xor = htonl(g_infile_length ^ ~0UL);
    l_fih.crc = htonl(g_infile_crc);
    l_fih.crc_xor = htonl(g_infile_crc ^ ~0UL);
    memcpy(g_buff + 8, &l_fih, sizeof(fileinfo_header));

    // copy data into first block; zero it then read from infile
//    memset(g_buff + 8 + sizeof(fileinfo_header), 0, g_1stblock_capacity);
    res = read(g_infile_fd, g_buff + 8 + sizeof(fileinfo_header), g_1stblock_capacity);
    if (res == 0) {
        // zero length file, nothing to do!
        return;
    }
    if (res < 0) {
        // actual error
        fprintf(stderr, "rsa: unable to read from input file during encrypt operation: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (res < g_1stblock_capacity) {
        // must be a really short file
        lastblock = 1;
    }
    if (g_debug > 0) {
        printf("do_encrypt: first block (fileinfo_header + %d used of initial data capacity of %d bytes)", res, g_1stblock_capacity);
        print_hex(g_buff, g_block_size);
    }

    mpz_t l_block;
    mpz_init(l_block);
    mpz_t l_cipher;
    mpz_init(l_cipher);
    mpz_t l_e;
    mpz_init(l_e);
    mpz_t l_n;
    mpz_init(l_n);
    size_t l_written;

    // load our key data
    mpz_import(l_e, 4, 1, sizeof(unsigned char), 0, 0, g_e);
    mpz_import(l_n, g_block_size, 1, sizeof(unsigned char), 0, 0, g_n);

    // load up our 1st block we just created
    mpz_import(l_block, g_block_size, 1, sizeof(unsigned char), 0, 0, g_buff);

    // and encrypt it
    mpz_powm(l_cipher, l_block, l_e, l_n);
    if (g_debug > 0) {
        gmp_printf("n      = %Zx\ne      = %Zx\nblock  = %Zx\ncipher = %Zx\n", l_n, l_e, l_block, l_cipher);
    }

    // and export it to aux block
    mpz_export(g_buff2, &l_written, 1, sizeof(unsigned char), 0, 0, l_cipher);
    if (l_written != g_block_size) {
        right_justify(l_written, g_block_size - l_written, (char *)g_buff2);
    }
    if (g_debug > 0) {
        printf("do_encrypt: first block (encrypted)");
        print_hex(g_buff2, g_block_size);
    }

    // write it to output file
    res = write(g_outfile_fd, g_buff2, g_block_size);
    if (res < 0) {
        fprintf(stderr, "rsa: unable to write to output file during encrypt operation: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (res < g_block_size) {
        // lol what? didn't write the whole block?
        fprintf(stderr, "rsa: unable to write entire block size of %d bytes to output file during encrypt operation\n", g_block_size);
    }

    // test our encryption (if d is loaded and debug flag is on)
    if ((g_d_loaded > 0) && (g_debug > 0)) {
        mpz_t l_d;
        mpz_init(l_d);
        mpz_import(l_d, g_block_size, 1, sizeof(unsigned char), 0, 0, g_d);
        mpz_t l_decrypted;
        mpz_init(l_decrypted);
        mpz_powm(l_decrypted, l_cipher, l_d, l_n);
        gmp_printf("decr.  = %Zx\n", l_decrypted);
        mpz_export(g_buff2, &l_written, 1, sizeof(unsigned char), 0, 0, l_decrypted);
        if (l_written != g_block_size) {
            right_justify(l_written, g_block_size - l_written, (char *)g_buff2);
        }
        printf("do_encrypt: first block (decrypted)");
        print_hex(g_buff2, g_block_size);
        mpz_clear(l_d);
        mpz_clear(l_decrypted);
    }

    // now do the same for all the rest of the data
    while (lastblock == 0) {
        // prepare block
        l_block_ctr++;
        get_random(g_buff, g_block_size);
        // padding
        g_buff[0] = 0;
        // copy data into block
//        memset(g_buff + 8, 0, g_block_capacity);
        res = read(g_infile_fd, g_buff + 8, g_block_capacity);
        if (res == 0) {
            // at the EOF, so don't make any new blocks
            lastblock = 1;
            continue;
        }
        if (res < 0) {
            // actual error
            fprintf(stderr, "rsa: unable to read from input file during encrypt operation: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        if (res < g_block_capacity) {
            lastblock = 1;
        }
        if (g_debug > 0) {
            printf("\ndo_encrypt: block #%d - %d used of block data capacity of %d bytes)", l_block_ctr, res, g_block_capacity);
            print_hex(g_buff, g_block_size);
        }
        // load up our 1st block we just created
        mpz_import(l_block, g_block_size, 1, sizeof(unsigned char), 0, 0, g_buff);

        // and encrypt it
        mpz_powm(l_cipher, l_block, l_e, l_n);
        if (g_debug > 0) {
            gmp_printf("n      = %Zx\ne      = %Zx\nblock  = %Zx\ncipher = %Zx\n", l_n, l_e, l_block, l_cipher);
        }
        // and export it to aux block
        mpz_export(g_buff2, &l_written, 1, sizeof(unsigned char), 0, 0, l_cipher);
        if (l_written != g_block_size) {
            right_justify(l_written, g_block_size - l_written, (char *)g_buff2);
        }
        if (g_debug > 0) {
            printf("do_encrypt: block (encrypted)");
            print_hex(g_buff2, g_block_size);
        }
        // write it to output file
        res = write(g_outfile_fd, g_buff2, g_block_size);
        if (res < 0) {
            fprintf(stderr, "rsa: unable to write to output file during encrypt operation: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        if (res < g_block_size) {
            // lol what? didn't write the whole block?
            fprintf(stderr, "rsa: unable to write entire block size of %d bytes to output file during encrypt operation\n", g_block_size);
        }
        // test our encryption (if d is loaded and debug flag is on)
        if ((g_d_loaded > 0) && (g_debug > 0)) {
            mpz_t l_d;
            mpz_init(l_d);
            mpz_import(l_d, g_block_size, 1, sizeof(unsigned char), 0, 0, g_d);
            mpz_t l_decrypted;
            mpz_init(l_decrypted);
            mpz_powm(l_decrypted, l_cipher, l_d, l_n);
            gmp_printf("decr.  = %Zx\n", l_decrypted);
            mpz_export(g_buff2, &l_written, 1, sizeof(unsigned char), 0, 0, l_decrypted);
            if (l_written != g_block_size) {
                right_justify(l_written, g_block_size - l_written, (char *)g_buff2);
            }
            printf("do_encrypt: block (decrypted)");
            print_hex(g_buff2, g_block_size);
            mpz_clear(l_d);
            mpz_clear(l_decrypted);
        }
    }

    mpz_clear(l_block);
    mpz_clear(l_cipher);
    mpz_clear(l_e);
    mpz_clear(l_n);
}

int main(int argc, char **argv)
{
    unsigned int i;
    int res; // result variable for UNIX reads
    int opt;
    while ((opt = getopt_long(argc, argv, "i:o:k:edsv?tw", g_options, NULL)) != -1) {
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
            case 'w':
            {
                g_outfile_overwrite = 1;
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
		printf("  -w (--overwrite) force overwrite of existing output file\n");
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

    // prepare urandom
    g_urandom_fd = open("/dev/urandom", O_RDONLY);
    if (g_urandom_fd < 0) {
        fprintf(stderr, "rsa: problems opening /dev/urandom: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
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
            prepare_infile();
            get_infile_crc();
            if (g_outfile_specified == 0) {
                fprintf(stderr, "rsa: this function requires that you specify an output file.\n");
                exit(EXIT_FAILURE);
            }
            prepare_outfile();
            do_encrypt();
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
            prepare_infile();
            if (g_infile_block_multiple > 0) {
                fprintf(stderr, "rsa: input file must be a multiple of block size to decrypt.\n");
                exit(EXIT_FAILURE);
            }
            if (g_outfile_specified == 0) {
                fprintf(stderr, "rsa: this function requires that you specify an output file.\n");
                exit(EXIT_FAILURE);
            }
            prepare_outfile();
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
            prepare_infile();
            get_infile_crc();
            if (g_outfile_specified == 0) {
                fprintf(stderr, "rsa: this function requires that you specify an output file.\n");
                exit(EXIT_FAILURE);
            }
            prepare_outfile();
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
            prepare_infile();
            if (g_infile_block_multiple > 0) {
                fprintf(stderr, "rsa: input file must be a multiple of block size to verify.\n");
                exit(EXIT_FAILURE);
            }
            if (g_outfile_specified == 0) {
                fprintf(stderr, "rsa: this function requires that you specify an output file.\n");
                exit(EXIT_FAILURE);
            }
            prepare_outfile();
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

    close(g_infile_fd);
    close(g_outfile_fd);

    return 0;
}
