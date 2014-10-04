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

#define GREATEST_STDOUT stderr
#include "greatest.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <vector>

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include "arithmetic.h"
#include "opensieve.h"

using namespace opensieve;

#define PATTERN_SEGMENT_SIZE 1024

#define PERFORMANCE_TEST 1

uint64_t global_sum = 0;
uint64_t global_cnt = 0;

/************************************************************************************/
TEST assembly_test()
{
    uint64_t a_3 = 0x2492492492492492;
    uint64_t a_5 = 0x4210842108421084;
    uint64_t a_7 = 0x810204081020408;
    uint64_t a_11 = 0x1002004008010020;
    uint64_t a_13 = 0x400200100080040;
    uint64_t a_17 = 0x800040002000100;
    uint64_t a_19 = 0x800010000200;

    uint64_t acc1 = 0;
    uint64_t acc2 = 0;
    uint64_t c = 0;

    acc1 |= jkk_asm_ror(a_3, 1);
    acc1 |= jkk_asm_rol(a_5, 1);
    acc1 |= jkk_asm_ror(a_7, 1);
    acc1 |= jkk_asm_rol(a_11, 2);
    acc1 |= jkk_asm_rol(a_13, 1);
    acc1 |= jkk_asm_rol(a_17, 4);
    acc1 |= jkk_asm_ror(a_19, 7);

    MASK_R(acc2, c, 3, a_3, 1);
    MASK_L(acc2, c, 5, a_5, 1);
    MASK_R(acc2, c, 7, a_7, 1);
    MASK_L(acc2, c, 11, a_11, 2);
    MASK_L(acc2, c, 13, a_13, 1);
    MASK_L(acc2, c, 17, a_17, 4);
    MASK_R(acc2, c, 19, a_19, 7);

    ASSERT_EQ(acc1, acc2)
    ;
    PASS()
    ;
}

/************************************************************************************/
void show_pattern(int prime)
{
    char char_table[PATTERN_SEGMENT_SIZE];

    memset(char_table, 0x0, PATTERN_SEGMENT_SIZE * sizeof(char));
    for (int64_t j = (prime * prime - 1) / 2; j < PATTERN_SEGMENT_SIZE; j += prime)
    {
        char_table[j] = 1;
    }

    uint64_t hex = 0;
    for (uint64_t i = 0; i < PATTERN_SEGMENT_SIZE; i++)
    {
        hex <<= 1;
        hex += char_table[i];
        printf("%d", char_table[i]);
        if (i % 64 == 63)
        {
            printf("\t %" PRIu64 "x\n", hex);
            hex = 0;
        }

    }
    printf("\n");
}

/************************************************************************************/
void old_sieve(uint64_t limit)
{
    uint64_t sqrt_limit = sqrt((double) limit) + 2;

    // generate small primes <= sqrt < 2^32
    char *small_primes = (char*) valloc((limit - 1) / 2 * sizeof(char));
    if (small_primes == 0)
    {
        fprintf(stderr, "Unable to allocate small_primes!\n");
        return;
    }

    memset(small_primes, 0xffffffff, ((limit - 1) / 2) * sizeof(char));
    for (uint64_t i = 3; i <= sqrt_limit; i += 2)
    {
        if (small_primes[(i - 1) / 2])
        {
            for (int64_t j = i; i * j <= limit; j += 2)
            {
                small_primes[(i * j - 1) / 2] = 0;
            }
        }
    }

    // calculate the exact number of primes for allocation
    uint64_t counter = 0;
    for (uint64_t i = 1; i < (limit - 1) / 2; i++)
    {
        if (small_primes[i])
        {
            // printf("B %" PRIu64 "u\n", i*2+1);
            counter++;
        }
    }
    printf("primes: %" PRIu64 "u\n", counter);
}

/************************************************************************************/
static void print_prime(uint64_t prime)
{
#if PERFORMANCE_TEST == 1
    prime++;
#else
    printf("%" PRIu64 "\n", prime);
#endif
}

/************************************************************************************/
static void hash_func(uint64_t prime)
{
    global_sum += prime;
    global_cnt++;
}

/************************************************************************************/
TEST simple_sieve_test()
{
    uint64_t hash_results[][3] =
    {
    { 1000000, 37550402023, 78498 },
    { 1234567, 56381686561, 95360 },
    { 8356428, 2263591463619, 562166 },
    { 15485864, 7472966967499, 1000000 },
    { 127, 1720, 31 },
    { 0, 0, 0 } };

    for (int i = 0; hash_results[i][0] != 0; i++)
    {
        global_sum = 0;
        global_cnt = 0;

        uint64_t *table = 0;
        uint64_t table_size;

        sieve_small(hash_results[i][0], &table, table_size);
        opensieve::process_primes(hash_func, table, table_size, 0);

        free(table);

        // printf("global_sum = %" PRIu64 "u global_cnt = %" PRIu64 "u\n",
        // global_sum, global_cnt);

        ASSERT_EQ(global_sum, hash_results[i][1])
        ;
        ASSERT_EQ(global_cnt, hash_results[i][2])
        ;
    }

    PASS()
    ;
    return 0;
}

/************************************************************************************/
SUITE(general_suite)
{
    RUN_TEST(assembly_test);
}

/************************************************************************************/
SUITE(sieve_suite)
{
    RUN_TEST(simple_sieve_test);
}

/************************************************************************************/
int devel_tests(void)
{
#if PERFORMANCE_TEST
    opensieve::sieve(0, 500, print_prime);
#else
    opensieve::sieve(0, 100, print_prime);
#endif
    return 0;
}

/************************************************************************************/
TEST devel_tests_case()
{
    devel_tests();
    PASS()
    ;
}

/************************************************************************************/
SUITE(devel_tests_suite)
{
    RUN_TEST(devel_tests_case);
}

/* Add definitions that need to be in the test runner's main file. */
GREATEST_MAIN_DEFS()
;

/************************************************************************************/
int main(int argc, char **argv)
{
    GREATEST_MAIN_BEGIN()
    ; /* command-line arguments, initialization. */
    RUN_SUITE(general_suite);
    RUN_SUITE(sieve_suite);
    RUN_SUITE(devel_tests_suite);
    GREATEST_MAIN_END(); /* display results */
}

