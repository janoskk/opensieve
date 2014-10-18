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
#include <string>
#include <stdlib.h>
#include <math.h>
#include <stdexcept>

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include "arithmetic.h"
#include "opensieve.h"

#ifndef SEGMENT_BITS
#define SEGMENT_BITS 20
#endif

#ifndef SEGMENT_SIZE
#define SEGMENT_SIZE (1ULL<<SEGMENT_BITS)
#endif

#ifndef OPEN_SIEVE_VERSION
#define OPEN_SIEVE_VERSION 2
#endif

#ifndef SEGMENTED_SIEVE_WITH_MASKING
#define SEGMENTED_SIEVE_WITH_MASKING 1
#endif

#ifndef SEGMENTED_SIEVE_VERSION
#define SEGMENTED_SIEVE_VERSION 1
#endif

#ifndef USE_ASM_VERSION_OF_MASKING
#define USE_ASM_VERSION_OF_MASKING 1
#endif

#ifndef USE_RECURSIVE_SIEVE
#define USE_RECURSIVE_SIEVE 1
#endif

#ifndef RECURSIVE_SIEVE_LIMIT
#define RECURSIVE_SIEVE_LIMIT (1ULL << 16)
#endif


namespace opensieve
{
char wheel30[] =
{ 1, 6, 5, 4, 3, 2, 1, 4, 3, 2, 1, 2, 1, 4, 3, 2, 1, 2, 1, 4, 3, 2, 1, 6, 5, 4, 3, 2, 1, 2 };

char rem30[] =
{ 1, 7, 11, 13, 17, 19, 23, 29 };
char wow30[] =
{ 6, 4, 2, 4, 2, 4, 6, 2 };
char inv30[] =
{ -1, 0, -1, -1, -1, -1, -1, 1, -1, -1, -1, 2, -1, 3, -1, -1, -1, 4, -1, 5, -1, -1, -1, 6, -1, -1, -1, -1, -1, 7 };

/************************************************************************************/
#define IS_PRIME_AT(is_prime, x) {                              \
	uint32_t seg = x >> 6;                                      \
	uint64_t off = jkk_asm_shl(1ULL,  /* 0x3f - */ (x & 0x3f)); \
	is_prime = ((*table)[seg] & off) == 0;                      \
}

/************************************************************************************/
#define SIEVE_AT(x, buf) {                                      \
	seg = pos >> 6;                                             \
	off = jkk_asm_shl(1ULL,  (pos & 0x3f));                     \
	buf[seg] |= off;                                            \
	pos += pos30[x];                                            \
	j++;                                                        \
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
        uint64_t acc = 0; // !!
        uint64_t c = 0;

        MASK_L(acc, c, 3, a_3, 2);
        MASK_L(acc, c, 5, a_5, 1);
        MASK_L(acc, c, 7, a_7, 6);
        MASK_L(acc, c, 11, a_11, 2);
        MASK_L(acc, c, 13, a_13, 1);
        MASK_L(acc, c, 17, a_17, 4);
        MASK_R(acc, c, 19, a_19, 7);

        table[i] = acc;
    }
}

/************************************************************************************/
void sieve_small(uint64_t limit, uint64_t **table, uint64_t& table_size)
{
    table_size = (((limit + 1) >> 1) + 63) >> 6;

    uint64_t no_of_segments = table_size / (SEGMENT_SIZE >> 7) + 1;
    uint64_t table_size_for_alloc = no_of_segments * (SEGMENT_SIZE >> 7);

    *table = (uint64_t*) valloc(table_size_for_alloc * sizeof(uint64_t));

    if (!*table)
    {
        throw new std::runtime_error("Unable to allocate memory!");
    }

    printf("Allocated %llu byte (%lluMB) memory.\n", (table_size * sizeof(uint64_t)),
            (table_size * sizeof(uint64_t)) >> 20);

#if USE_RECURSIVE_SIEVE == 1
    if (limit > RECURSIVE_SIEVE_LIMIT)
    {
        printf("small sieve table is still too large\n");
        sieve_segments(0, no_of_segments, 0, *table);
        return;
    }
#endif

    const uint64_t sqrt_limit = sqrt((double) limit) + 1;

    //    printf("limit      = %" PRIu64 "u\n", limit);
    //    printf("sqrt_limit = %" PRIu64 "u\n", sqrt_limit);
    //    printf("table_size = %" PRIu64 "u\n", table_size);

#if USE_ASM_VERSION_OF_MASKING == 1
    asm_masking(*table, table_size, 0);
#else
    c_masking(*table, table_size, 0);
#endif

    (*table)[0] &= 0xffffffffffffffff ^ (2 | 4 | 8 | 32 | 64 | 256 | 512); // correcting 3, 5, 7, 11, 13, 17, 19

    for (uint64_t i = 23; i <= sqrt_limit; i += 2)
    {
        int is_prime = 0;
        IS_PRIME_AT(is_prime, (i - 1) / 2);
        if (is_prime)
        {
#if OPEN_SIEVE_VERSION == 1
            for (uint64_t j = (i * i - 1) / 2; j <= limit / 2; j += i)
            {
                uint32_t seg = j >> 6;
                uint64_t off = jkk_asm_shl(1ULL, (j & 0x3f));
                (*table)[seg] |= off;
            }
#elif OPEN_SIEVE_VERSION == 2
            uint64_t pos = (i * i - 1) / 2;
            for (uint64_t j = i; pos <= limit / 2; j += wheel30[j % 30])
            {
                uint32_t seg = pos >> 6;
                uint64_t off = jkk_asm_shl(1ULL, (pos & 0x3f));
                (*table)[seg] |= off;
                pos = (i * j - 1) / 2;
            }
#elif OPEN_SIEVE_VERSION == 3
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
            pos30[0] = (wow30[j++ & 7] >> 1) * i;
            pos30[1] = (wow30[j++ & 7] >> 1) * i;
            pos30[2] = (wow30[j++ & 7] >> 1) * i;
            pos30[3] = (wow30[j++ & 7] >> 1) * i;
            pos30[4] = (wow30[j++ & 7] >> 1) * i;
            pos30[5] = (wow30[j++ & 7] >> 1) * i;
            pos30[6] = (wow30[j++ & 7] >> 1) * i;
            pos30[7] = (wow30[j++ & 7] >> 1) * i;

            while (pos <= limit / 2 - 15 * i)
            {
                uint32_t seg;
                uint64_t off;
                SIEVE_AT(0, (*table));
                SIEVE_AT(1, (*table));
                SIEVE_AT(2, (*table));
                SIEVE_AT(3, (*table));
                SIEVE_AT(4, (*table));
                SIEVE_AT(5, (*table));
                SIEVE_AT(6, (*table));
                SIEVE_AT(7, (*table));
            }

            for (int k = 0; pos <= limit / 2; k++)
            {
                uint32_t seg;
                uint64_t off;
                SIEVE_AT(k & 7, (*table));
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
uint64_t process_primes(SIEVE_PROCESS_FUNC *process_for_primes, uint64_t *table, uint64_t table_size,
        unsigned current_segment)
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
                    prime = ((uint64_t) current_segment * SEGMENT_SIZE) + (((i << 6) + off) << 1) + 1;
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
static uint64_t get_diff(uint64_t *table, uint64_t table_size, uint64_t &seg, uint64_t &pos)
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
void c_masking(uint64_t table[], unsigned length, unsigned table_offset)
{
    uint64_t a_3_11 = 0x2492492492492492 | 0x1002004008010020;
    uint64_t a_5_7 = 0x4210842108421084 | 0x810204081020408;
    uint64_t a_13 = 0x400200100080040;
    uint64_t a_17 = 0x800040002000100;
    uint64_t a_19 = 0x800010000200;

    register uint64_t acc = 0; // !!
    register uint64_t c = 0;

    for (unsigned i = 0; i < table_offset % 33; i++)
        MASK_L(acc, c, 33, a_3_11, 2);
    for (unsigned i = 0; i < table_offset % 35; i++)
        MASK_L(acc, c, 35, a_5_7, 6);
    for (unsigned i = 0; i < table_offset % 13; i++)
        MASK_L(acc, c, 13, a_13, 1);
    for (unsigned i = 0; i < table_offset % 17; i++)
        MASK_L(acc, c, 17, a_17, 4);
    for (unsigned i = 0; i < table_offset % 19; i++)
        MASK_R(acc, c, 19, a_19, 7);

    uint64_t b = a_3_11;
    b |= a_5_7;
    b |= a_13;
    b |= a_17;
    b |= a_19;
    table[0] = b;

    for (unsigned i = 1; i < length; i++)
    {
        uint64_t acc = 0; // !!
        uint64_t c = 0;

        MASK_L(acc, c, 33, a_3_11, 2);
        MASK_L(acc, c, 35, a_5_7, 6);
        MASK_L(acc, c, 13, a_13, 1);
        MASK_L(acc, c, 17, a_17, 4);
        MASK_R(acc, c, 19, a_19, 7);

        table[i] = acc;
    }
}

/************************************************************************************/
void sieve_segments(uint64_t first_segment, uint64_t no_of_segments, SIEVE_PROCESS_FUNC *process_for_primes,
        uint64_t *table)
{
    //uint64_t first = first_segment * SEGMENT_SIZE + 1;
    uint64_t last = (first_segment + no_of_segments) * SEGMENT_SIZE;
    uint64_t sqrt_of_last_elem = (int64_t) sqrt((double) last + SEGMENT_SIZE) + 2;

    uint64_t *small_primes;
    uint64_t small_primes_size;
    sieve_small(sqrt_of_last_elem, &small_primes, small_primes_size);

    uint64_t *segment;
    if (table != 0)
    {
        segment = table;
    }
    else
    {
        segment = (uint64_t*) malloc((SEGMENT_SIZE >> 7) * sizeof(uint64_t));
    }

    for (uint64_t segment_no = first_segment; segment_no < first_segment + no_of_segments; segment_no++)
    {
        uint64_t segment_first = segment_no * SEGMENT_SIZE + 1;
        uint64_t segment_last = segment_no * SEGMENT_SIZE + SEGMENT_SIZE;
        uint64_t seg = 0;
        uint64_t diff = 0;
#if SEGMENTED_SIEVE_WITH_MASKING
        uint64_t prime = 19;
        uint64_t pos = 512;
#if USE_ASM_VERSION_OF_MASKING == 1
        asm_masking(segment, SEGMENT_SIZE >> 7, (segment_no) * (SEGMENT_SIZE >> 7));
#else
        c_masking(segment, SEGMENT_SIZE >> 7, (segment_no) * (SEGMENT_SIZE >> 7));
#endif
        if (segment_no == 0)
        {
            segment[0] &= 0xffffffffffffffff ^ (2 | 4 | 8 | 32 | 64 | 256 | 512); // correcting 3, 5, 7, 11, 13, 17, 19
        }

        while ((diff = get_diff(small_primes, small_primes_size, seg, pos)) > 0)
#else
        memset(segment, 0x0, (SEGMENT_SIZE >> 7) * sizeof(uint64_t));
        uint64_t prime = 1;
        uint64_t pos = 1;
        while ((diff = get_diff(small_primes, small_primes_size, seg, pos)) > 0)
#endif
        {
            prime += diff;

            uint64_t next = prime * prime;
            if (next > segment_last)
            {
                continue;
            }

#if SEGMENTED_SIEVE_VERSION == 1
            if (segment_first > next) // almost always, except the smaller primes and in the beginning of sieve
            {
                next = ((segment_first) / prime) * prime;
                // This is wrong! Check the similar parts!
                // next = (next == first) ? next : next + prime; // to be on the safe side
                next = (next < segment_first) ? next + prime : next;
                next = ((next & 1) == 0) ? next + prime : next;
            }
            next = ((next - 1) >> 1) % ((SEGMENT_SIZE >> 1));
            while (next < SEGMENT_SIZE >> 1)
            {
                uint32_t seg = next >> 6;
                uint64_t off = jkk_asm_shl(1ULL, next & 0x3f);
                segment[seg] |= off;
                next += prime;
            }
#elif SEGMENTED_SIEVE_VERSION == 2
            if (segment_first > next) // almost always, except the smaller primes and in the beginning of sieve
            {
                next = (segment_first / prime) * prime; // ezt kene optolni j-hez! *prime nem kell idd
                // at this point, next always <= segment_first
                next = (next < segment_first) ? next + prime : next;
                next = ((next & 1) == 0) ? next + prime : next;
            }
            uint64_t pos = next;
            for (uint64_t j = next / prime; pos <= segment_last; j += wheel30[j % 30])
            {
                uint32_t seg = (((pos - 1) / 2) % (SEGMENT_SIZE >> 1)) >> 6;
                uint64_t off = jkk_asm_shl(1ULL, (((pos - 1) / 2) & 0x3f));
                segment[seg] |= off;
                pos = prime * j;
            }
#elif SEGMENTED_SIEVE_VERSION == 3
            uint64_t multi = prime;
            if (segment_first > next) // almost always, except the smaller primes and in the beginning of sieve
            {
                multi = segment_first / prime;
                next = (segment_first / prime) * prime;
                // at this point, next always <= segment_first
                multi = (next < segment_first) ? multi + 1 : multi;
                next = (next < segment_first) ? next + prime : next;
                multi = ((next & 1) == 0) ? multi + 1 : multi;
                next = ((next & 1) == 0) ? next + prime : next;
            }
            uint64_t pos = (next - 1) >> 1;
            for (uint64_t j = multi; pos <= segment_last >> 1; j += wheel30[j % 30])
            {
                uint32_t seg = (pos & ((SEGMENT_SIZE >> 1) - 1)) >> 6;
                uint64_t off = jkk_asm_shl(1ULL, (pos & 0x3f));
                segment[seg] |= off;
                pos = (prime * j - 1) >> 1;
            }
#endif
        }

        // n is the last sieving prime in that segment
        /* n = */process_primes(process_for_primes, segment, SEGMENT_SIZE >> 7, segment_no);

        if (table != 0)
        {
            segment += (SEGMENT_SIZE >> 7);
        }
    }

    free(small_primes);
}

/************************************************************************************/
void sieve(uint64_t first_number, uint64_t last_number, SIEVE_PROCESS_FUNC *process_for_primes)
{
    uint64_t first_segment = first_number / (SEGMENT_SIZE << 1);
    uint64_t no_of_segments = last_number / (SEGMENT_SIZE << 1) - first_segment + 1;
    sieve_segments(first_segment, no_of_segments, process_for_primes);
}

}

