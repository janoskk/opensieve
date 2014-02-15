#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "opensieve.h"

#ifndef SEGMENT_BITS
  #define SEGMENT_BITS 15
#endif
#define SEGMENT_SIZE (1<<SEGMENT_BITS)
//#define SEGMENT_SIZE 1000000000


char wheel30[] =
    { 1, 6, 5, 4, 3, 2, 1,
      4, 3, 2, 1, 2, 1,
      4, 3, 2, 1, 2, 1,
      4, 3, 2, 1, 6, 5, 4, 3, 2, 1, 2 };

char rem30[] = { 1,  7, 11, 13, 17, 19, 23, 29 };
char wow30[] = { 6,  4,  2,  4,  2,  4,  6,  2 };
char inv30[] = { -1, 0, -1, -1, -1, -1, -1, 1, -1, -1,
                 -1, 2, -1, 3, -1, -1, -1, 4, -1, 5,
                 -1, -1, -1, 6, -1, -1, -1, -1, -1, 7 };

/************************************************************************************/
/*
 * Replacement for num >> bits computation what is limited to shift maximum 31 bits
 */
inline uint64_t jkk_asm_shl(uint64_t num, int bits)
{
    __asm__("shlq %%cl, %%rax;"
        : "=a" (num)
                    : "a" (num), "c" (bits)
                   );
    return num;
}


/************************************************************************************/
/*
 * Replacement for num << bits computation what is limited to shift maximum 31 bits
 */
inline uint64_t jkk_asm_shr(uint64_t num, int bits)
{
    __asm__("shrq %%cl, %%rax;"
        : "=a" (num)
                    : "a" (num), "c" (bits)
                   );
    return num;
}


/************************************************************************************/
inline uint64_t jkk_asm_bsf(uint64_t num)
{
    uint64_t ret = 0;
    asm("bsf %1, %0;"
    :"=r" (ret)     /* output */
                :"r" (num)      /* input */);
    return ret;
}

#define MASK_R(num, num_var, count)   \
{                                     \
        num_var >>= count;                \
        c = num_var;                  \
        c <<= num;                    \
        num_var |= c;                 \
        acc |= num_var;               \
}

#define MASK_L(num, num_var, count)   \
{                                     \
        num_var <<= count;                \
        c = num_var;                  \
        c >>= num;                    \
        num_var |= c;                 \
        acc |= num_var;               \
}


/************************************************************************************/
void bitsieve(uint64_t table[], unsigned length)
{
    uint64_t a_3 = 0x2492492492492492;
    uint64_t a_5 = 0x4210842108421084;
    uint64_t a_7 = 0x810204081020408;
    uint64_t a_11 = 0x1002004008010020;
    uint64_t a_13 = 0x400200100080040;
    uint64_t a_17 = 0x800040002000100;
    uint64_t a_19 = 0x800010000200;

    uint64_t b = a_3;
    b |= a_5;
    b |= a_7;
    b |= a_11;
    b |= a_13;
    b |= a_17;
    b |= a_19;
    table[0] = b;

    for (unsigned i = 1; i < length; i++)
    {
        register uint64_t c = 0;
        register uint64_t acc = 0; // !!

        MASK_R(3, a_3, 1);
        MASK_L(5, a_5, 1);
        MASK_R(7, a_7, 1);
        MASK_L(11, a_11, 2);
        MASK_L(13, a_13, 1);
        MASK_L(17, a_17, 4);
        MASK_R(19, a_19, 7);

        table[i] = acc;
    }
}


/************************************************************************************/
#define IS_PRIME_AT(is_prime, x) {					\
	uint32_t seg = x >> 6;						\
	uint64_t off = jkk_asm_shl(1ULL,  /* 0x3f - */ (x & 0x3f));	\
	is_prime = ((*table)[seg] & off) == 0;				\
    }


/************************************************************************************/
#define SIEVE_AT(x) {					\
	seg = pos >> 6;					\
	off = jkk_asm_shl(1ULL,  (pos & 0x3f));		\
	(*table)[seg] |= off;				\
	pos += pos30[x];				\
	j++;						\
    }


/************************************************************************************/
void open_sieve(uint64_t limit, uint64_t **table, uint64_t& table_size)
{
    table_size = (((limit + 1) >> 1) + 63) >> 6;
    *table = (uint64_t*)valloc(table_size * sizeof(uint64_t));
    const uint64_t sqrt_limit = sqrt((double)limit) + 1;

    /*
    printf("limit      = %llu\n", limit);
    printf("sqrt_limit = %llu\n", sqrt_limit);
    printf("table_size = %llu\n", table_size);
    */

    bitsieve(*table, table_size);
    (*table)[0] &= 0xffffffffffffffff ^ (2|4|8|32|64|256|512); // correcting 3, 5, 7, 11, 13, 17, 19

    for (uint64_t i = 17; i <= sqrt_limit; i+=2)
    {
        int is_prime = 0;
        IS_PRIME_AT(is_prime, (i-1)/2);
        if (is_prime)
        {
#if VERSION == 1
            for (uint64_t j = (i * i - 1) / 2; j <= limit/2; j += i)
            {
                uint32_t seg = j >> 6;
                uint64_t off = jkk_asm_shl(1ULL,  (j & 0x3f));
                table[seg] |= off;
            }
#elif VERSION == 2
            uint64_t pos = (i * i - 1) / 2;
            for (uint64_t j = i; pos <= limit/2; j += wheel30[j % 30])
            {
                uint32_t seg = pos >> 6;
                uint64_t off = jkk_asm_shl(1ULL,  (pos & 0x3f));
                table[seg] |= off;
                pos = (i * j - 1) / 2;
            }
#elif VERSION == 3
            for (int j = 0; j < 8; j++)
            {
                wow30prime[j] = (wow30[j] >> 1) * i;
            }

            uint64_t pos = (i * i - 1) / 2;
            int64_t j = inv30[i % 30];
            if (j == -1)
            {
                fprintf(stderr, "It should not be happened! j = -1 at prime=%llu\n", i);
                continue;
            }
            while ( pos <= limit/2 )
            {
                uint32_t seg = pos >> 6;
                uint64_t off = jkk_asm_shl(1ULL,  (pos & 0x3f));
                table[seg] |= off;
                pos += wow30prime[ j & 7 ];
                j++;
            }
#else
            uint64_t pos = (i * i - 1) / 2;
            int64_t j = inv30[i % 30];
            if (j == -1)
            {
                fprintf(stderr, "It should not be happened! j = -1 at prime=%llu\n", i);
                continue;
            }

            static uint64_t pos30[8];
            pos30[0] = (wow30[ j++ & 7 ] >> 1) * i;
            pos30[1] = (wow30[ j++ & 7 ] >> 1) * i;
            pos30[2] = (wow30[ j++ & 7 ] >> 1) * i;
            pos30[3] = (wow30[ j++ & 7 ] >> 1) * i;
            pos30[4] = (wow30[ j++ & 7 ] >> 1) * i;
            pos30[5] = (wow30[ j++ & 7 ] >> 1) * i;
            pos30[6] = (wow30[ j++ & 7 ] >> 1) * i;
            pos30[7] = (wow30[ j++ & 7 ] >> 1) * i;

            while (pos <= limit/2 - 15 * i)
            {
                uint32_t seg;
                uint64_t off;
                SIEVE_AT(0);
                SIEVE_AT(1);
                SIEVE_AT(2);
                SIEVE_AT(3);
                SIEVE_AT(4);
                SIEVE_AT(5);
                SIEVE_AT(6);
                SIEVE_AT(7);
            }

            for (int k = 0; pos <= limit/2; k++)
            {
                uint32_t seg;
                uint64_t off;
                SIEVE_AT(k & 7);
            }
#endif

        }
    }

    uint64_t unmask = 0xffffffffffffffff;
    uint64_t rem = ((limit + 1) >> 1) & 0x3f;
    if (rem != 0)
    {
        unmask = jkk_asm_shr(unmask, rem);
        unmask = jkk_asm_shl(unmask, rem);
        (*table)[table_size - 1] |= unmask;
    }
}


/************************************************************************************/
void process_primes(SIEVE_PROCESS_FUNC *process_for_primes,
                    uint64_t *table,
                    uint64_t table_size)
{
    uint64_t counter = 0;
    for(uint64_t i = 0; i < table_size; i++)
    {
        uint64_t num = table[i];
        if (num != 0)
        {
            int off = 0;
            for (uint64_t pos = 1; pos != 0; pos <<= 1)
            {
                if ((num & pos) != pos)
                {
                    uint64_t prime = (((i << 6) + off) << 1) + 1;
                    prime = (prime == 1) ? 2 : prime;
                    if (process_for_primes != 0)
                    {
                        (*process_for_primes)(prime);
                    }
                    counter++;
                }
                off++;
            }
        }
    }
}


/************************************************************************************/
#if 0
static void print_prime(uint64_t prime)
{
    printf("%lld\n", prime);
}
#endif


/************************************************************************************/
int old_main(void)
{
    uint64_t *table = 0;
    uint64_t table_size;

    //open_sieve(1000037, &table, table_size);
    //process_primes(print_prime, table, table_size);

    open_sieve(1000000000, &table, table_size);
    process_primes(0, table, table_size);

    free(table);

    return 0;
}
