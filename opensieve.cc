/*
 *  Copyright 2014 by Janos Kasza
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include "arithmetic.h"
#include "opensieve.h"

#ifndef SEGMENT_BITS
#define SEGMENT_BITS 20
#endif
#define SEGMENT_SIZE (1<<SEGMENT_BITS)
//#define SEGMENT_SIZE 1000000000

char wheel30[] =
{ 1, 6, 5, 4, 3, 2, 1, 4, 3, 2, 1, 2, 1, 4, 3, 2, 1, 2, 1, 4, 3, 2, 1, 6, 5, 4,
        3, 2, 1, 2 };

char rem30[] =
{ 1, 7, 11, 13, 17, 19, 23, 29 };
char wow30[] =
{ 6, 4, 2, 4, 2, 4, 6, 2 };
char inv30[] =
{ -1, 0, -1, -1, -1, -1, -1, 1, -1, -1, -1, 2, -1, 3, -1, -1, -1, 4, -1, 5, -1,
        -1, -1, 6, -1, -1, -1, -1, -1, 7 };

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
        register uint64_t acc = 0; // !!
        register uint64_t c = 0;

        /*
         * TODO: maybe the masks can be improved like this:
         * tmp = a_17 >> (17-4);
         * a_17 = shld a_17, tmp, 4
         * */

        MASK_R(acc, c, 3, a_3, 1);
        MASK_L(acc, c, 5, a_5, 1);
        MASK_R(acc, c, 7, a_7, 1);
        MASK_L(acc, c, 11, a_11, 2);
        MASK_L(acc, c, 13, a_13, 1);
        MASK_L(acc, c, 17, a_17, 4);
        MASK_R(acc, c, 19, a_19, 7);

        table[i] = acc;
    }
}

/************************************************************************************/
void bitsieve(uint64_t table[], unsigned length, unsigned table_offset)
{
    uint64_t a_3 = 0x2492492492492492;
    uint64_t a_5 = 0x4210842108421084;
    uint64_t a_7 = 0x810204081020408;
    uint64_t a_11 = 0x1002004008010020;
    uint64_t a_13 = 0x400200100080040;
    uint64_t a_17 = 0x800040002000100;
    uint64_t a_19 = 0x800010000200;

    register uint64_t acc = 0; // !!
    register uint64_t c = 0;

    MASK_R(acc, c, 3, a_3, table_offset % 64);
    MASK_L(acc, c, 5, a_5, table_offset % 64);
    MASK_R(acc, c, 7, a_7, table_offset % 64);
    MASK_L(acc, c, 11, a_11, (2 * table_offset) % 64);
    MASK_L(acc, c, 13, a_13, table_offset % 64);
    MASK_L(acc, c, 17, a_17, (4 * table_offset) % 64);
    MASK_R(acc, c, 19, a_19, (7 * table_offset) % 64);

    table[0] = acc;

    for (unsigned i = 1; i < length; i++)
    {
        register uint64_t acc = 0; // !!
        register uint64_t c = 0;

        MASK_R(acc, c, 3, a_3, 1);
        MASK_L(acc, c, 5, a_5, 1);
        MASK_R(acc, c, 7, a_7, 1);
        MASK_L(acc, c, 11, a_11, 2);
        MASK_L(acc, c, 13, a_13, 1);
        MASK_L(acc, c, 17, a_17, 4);
        MASK_R(acc, c, 19, a_19, 7);

        table[i] = acc;
    }
}

/************************************************************************************/
#define IS_PRIME_AT(is_prime, x) {                              \
	uint32_t seg = x >> 6;                                      \
	uint64_t off = jkk_asm_shl(1ULL,  /* 0x3f - */ (x & 0x3f)); \
	is_prime = ((*table)[seg] & off) == 0;                      \
}

/************************************************************************************/
#define SIEVE_AT(x) {                                           \
	seg = pos >> 6;                                             \
	off = jkk_asm_shl(1ULL,  (pos & 0x3f));                     \
	(*table)[seg] |= off;                                       \
	pos += pos30[x];                                            \
	j++;                                                        \
}

/************************************************************************************/
void open_sieve(uint64_t limit, uint64_t **table, uint64_t& table_size)
{
    table_size = (((limit + 1) >> 1) + 63) >> 6;
    *table = (uint64_t*) valloc(table_size * sizeof(uint64_t));
    const uint64_t sqrt_limit = sqrt((double) limit) + 1;

    //    printf("limit      = %" PRIu64 "u\n", limit);
    //    printf("sqrt_limit = %" PRIu64 "u\n", sqrt_limit);
    //    printf("table_size = %" PRIu64 "u\n", table_size);

    bitsieve(*table, table_size);
    (*table)[0] &= 0xffffffffffffffff ^ (2 | 4 | 8 | 32 | 64 | 256 | 512); // correcting 3, 5, 7, 11, 13, 17, 19

    for (uint64_t i = 23; i <= sqrt_limit; i += 2)
    {
        int is_prime = 0;
        IS_PRIME_AT(is_prime, (i - 1) / 2);
        if (is_prime)
        {
#define VERSION 1
#if VERSION == 1
            for (uint64_t j = (i * i - 1) / 2; j <= limit / 2; j += i)
            {
                uint32_t seg = j >> 6;
                uint64_t off = jkk_asm_shl(1ULL, (j & 0x3f));
                (*table)[seg] |= off;
            }
#elif VERSION == 2
            uint64_t pos = (i * i - 1) / 2;
            for (uint64_t j = i; pos <= limit/2; j += wheel30[j % 30])
            {
                uint32_t seg = pos >> 6;
                uint64_t off = jkk_asm_shl(1ULL, (pos & 0x3f));
                (*table)[seg] |= off;
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
                fprintf(stderr, "It should not be happened! j = -1 at prime=%" PRIu64 "u\n", i);
                continue;
            }
            while ( pos <= limit/2 )
            {
                uint32_t seg = pos >> 6;
                uint64_t off = jkk_asm_shl(1ULL, (pos & 0x3f));
                (*table)[seg] |= off;
                pos += wow30prime[ j & 7 ];
                j++;
            }
#else
            uint64_t pos = (i * i - 1) / 2;
            int64_t j = inv30[i % 30];
            if (j == -1)
            {
                fprintf(stderr, "It should not be happened! j = -1 at prime=%" PRIu64 "u\n", i);
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
uint64_t process_primes(SIEVE_PROCESS_FUNC *process_for_primes, uint64_t *table,
        uint64_t table_size, unsigned current_segment)
{
    uint64_t prime = 0;
    for (uint64_t i = 0; i < table_size; i++)
    {
        uint64_t num = table[i];
        if (num != 0)
        {
            int off = 0;
            for (uint64_t pos = 1; pos != 0; pos <<= 1)
            {
                if ((num & pos) != pos)
                {
                    prime = (current_segment * SEGMENT_SIZE)
                            + (((i << 6) + off) << 1) + 1;
                    prime = (prime == 1) ? 2 : prime;
                    if (process_for_primes != 0)
                    {
                        (*process_for_primes)(prime);
                    }
                }
                off++;
            }
        }
    }
    return prime;
}

/************************************************************************************/
static uint64_t get_diff(uint64_t *table, uint64_t table_size, uint64_t &seg,
        uint64_t &pos)
{
    pos <<= 1;
    seg = (pos == 0) ? seg + 1 : seg;
    pos = (pos == 0) ? 1 : pos;
    uint64_t diff = 2;
    for (; seg < table_size; seg++)
    {
        uint64_t num = table[seg];
        for (; pos != 0; pos <<= 1)
        {
            if ((num & pos) != pos)
            {
                return diff;
            }
            diff += 2;
        }
        pos = 1;
    }
    return 0;
}

/************************************************************************************/
void segmented_sieve(int64_t first_segment, int no_of_segments,
        SIEVE_PROCESS_FUNC *process_for_primes)
{
    uint64_t first = first_segment * SEGMENT_SIZE + 1;
    uint64_t last = (first_segment + no_of_segments) * SEGMENT_SIZE;
    uint64_t sqrt_of_last_elem = (int64_t) sqrt((double) last + SEGMENT_SIZE) + 2;

    uint64_t *small_primes;
    uint64_t small_primes_size;
    open_sieve(sqrt_of_last_elem, &small_primes, small_primes_size);

    uint64_t segment[SEGMENT_SIZE >> 7];

    uint64_t n = (first == 1) ? 3 : first;
    uint64_t segment_no = first_segment;
    while (n < last)
    {
        memset(segment, 0x0, (SEGMENT_SIZE >> 7) * sizeof(uint64_t));
        uint64_t segment_first = segment_no * SEGMENT_SIZE + 1;
        uint64_t segment_last = segment_no * SEGMENT_SIZE + SEGMENT_SIZE;
        uint64_t seg = 0;
        uint64_t diff = 0;
#define WITH_MASK_SIEVE 0
#if WITH_MASK_SIEVE
        // TODO: test this code carefully!
        uint64_t prime = 19;
        uint64_t pos = 512;
        bitsieve(segment, SEGMENT_SIZE >> 7, segment_no * SEGMENT_SIZE);
        if (first_segment == 0)
        {
            segment[0] &= 0xffffffffffffffff
            ^ (2 | 4 | 8 | 32 | 64 | 256 | 512); // correcting 3, 5, 7, 11, 13, 17, 19
        }
        while ((diff = get_diff(small_primes, small_primes_size, seg, pos)) > 0)
#else
        uint64_t prime = 1;
        uint64_t pos = 1;
        while ((diff = get_diff(small_primes, small_primes_size, seg, pos)) > 0)
#endif
        {
            prime += diff;
            //printf("sieving with prime %" PRIu64 "\n", prime);

            uint64_t next = prime * prime;
            if (next > segment_last)
            {
                continue;
            }
            if (segment_first > next) // almost always, except the smaller primes and in the beginning of sieve
            {
                next = ((segment_first) / prime) * prime;
                // This is wrong! Check the similar parts!
                // next = (next == first) ? next : next + prime; // to be on the safe side
                next = (next < segment_first) ? next + prime : next;
                next = ((next & 1) == 0) ? next + prime : next;
            }
            //printf("prime: %llu next: %llu\n", prime, next);

            next = ((next - 1) >> 1) % (SEGMENT_SIZE >> 1);
            while (next < SEGMENT_SIZE >> 1)
            {
                uint32_t seg = next >> 6;
                uint64_t off = jkk_asm_shl(1ULL, next & 0x3f);
                segment[seg] |= off;

                next += prime;
            }
        }

        n = process_primes(process_for_primes, segment,
        SEGMENT_SIZE >> 7, segment_no);
        //printf("utolso prim ebben a segment-ben: %llu\n", n);

        segment_no++;
    }

}

