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

#include "arithmetic.h"
#include "opensieve.h"

#define TEST_FILE_TEMPLATE "test_results/test_%d_os.txt"

#define PATTERN_SEGMENT_SIZE 1024

#define PERFORMANCE_TEST 1
#define MASKING_TEST_LENGTH_1 4096
#define MASKING_TEST_LENGTH_2 (1<<13)

uint64_t global_sum = 0;
uint64_t global_cnt = 0;
FILE *global_file = 0;

/************************************************************************************/
static void print_prime_(uint64_t prime)
{
#if PERFORMANCE_TEST == 1
    prime++;
#else
    printf("%" PRIu64 "\n", prime);
#endif
}

/************************************************************************************/
static void write_prime(uint64_t prime)
{
    fprintf(global_file, "%" PRIu64 "\n", prime);
}

/************************************************************************************/
static void hash_func_write(uint64_t prime)
{
    fprintf(global_file, "prime: %" PRIu64 " global_sum: %"PRIu64" global_cnt: %" PRIu64 "\n", prime, global_sum,
            global_cnt);

    global_sum += prime;
    global_cnt++;
}

/************************************************************************************/
static void hash_func(uint64_t prime)
{
    global_sum += prime;
    global_cnt++;
}

/************************************************************************************/
TEST masking_test()
{
#define MASKING_TEST_LENGTH_1 4096
    uint64_t arr1[MASKING_TEST_LENGTH_1];
    uint64_t arr2[MASKING_TEST_LENGTH_1];

    for (unsigned j = 0; j < 10000; j++)
    {
        asm_masking(arr1, MASKING_TEST_LENGTH_1, j);
        opensieve::internal::c_masking(arr2, MASKING_TEST_LENGTH_1, j);

        for (unsigned i = 0; i < MASKING_TEST_LENGTH_1; i++)
        {
            if (arr1[i] != arr2[i])
            {
                printf("i=%u arr1[i]=%p arr2[i]=%p\n", i, (void*) (arr1[i]), (void*) (arr2[i]));
            }
            ASSERT_EQ(arr1[i], arr2[i])
            ;
        }
    }

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
        char filename[128];
        sprintf(filename, TEST_FILE_TEMPLATE, 11 + i);
        global_file = fopen(filename, "w");

        global_sum = 0;
        global_cnt = 0;

        uint64_t *table = 0;
        uint64_t table_size;

        opensieve::internal::sieve_small(hash_results[i][0], &table, table_size);
        opensieve::internal::process_primes(hash_func_write, table, table_size, 0 /* table */, 0 /* first_num */,
                hash_results[i][0] /* last_num */);

        free(table);
        fclose(global_file);

        //printf("global_sum = %" PRIu64 "u global_cnt = %" PRIu64 "u\n", global_sum, global_cnt);

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
TEST complex_sieve_test()
{
    uint64_t hash_results[][4] =
    {
    { 1, 127, 1720, 31 },
    { 1, 15485864, 7472966967499, 1000000 },
    { 123456789, 987654321, 23726506105551414ULL, 43224192ULL },
    { 10101, 101010101, 284713236839279ULL, 5814973ULL },
    { 1ULL << 20, 1ULL << 21, 115430379568ULL, 73586ULL },
    { (1ULL << 24) + 1, (1ULL << 25), 24754017328490ULL, 985818ULL },
    { (1ULL << 24) - 1, (1ULL << 25), 24754017328490ULL, 985818ULL },
    { (1ULL << 24) + 1, (1ULL << 25) + 1, 24754017328490ULL, 985818ULL },
    { (1ULL << 24) - 1, (1ULL << 25) + 1, 24754017328490ULL, 985818ULL },
    { (1ULL << 24) + 1, (1ULL << 25) - 1, 24754017328490ULL, 985818ULL },
    { (1ULL << 24) - 1, (1ULL << 25) - 1, 24754017328490ULL, 985818ULL },
    { 0, 0, 0 } };

    for (int i = 0; hash_results[i][0] != 0; i++)
    {
        global_sum = 0;
        global_cnt = 0;

        opensieve::sieve(hash_results[i][0], hash_results[i][1], hash_func);

        //printf("global_sum = %" PRIu64 "ULL global_cnt = %" PRIu64 "ULL\n", global_sum, global_cnt);

        ASSERT_EQ(global_sum, hash_results[i][2])
        ;
        ASSERT_EQ(global_cnt, hash_results[i][3])
        ;
    }

    PASS()
    ;
    return 0;
}

/************************************************************************************/
TEST file_sieve_test()
{
    char filename[128];
    sprintf(filename, TEST_FILE_TEMPLATE, 51);
    global_file = fopen(filename, "w");
    ASSERTm("File cannot be open in file_sieve_test!", global_file != NULL)
    ;

    opensieve::sieve(0, 10000000, write_prime);

    fclose(global_file);
    PASS()
    ;
}

/************************************************************************************/
TEST devel_tests_case()
{
    if (PERFORMANCE_TEST)
    {
        uint64_t a = 1ULL << 20 * 1ULL << 10;
        global_cnt = 0;
        global_sum = 0;

        opensieve::sieve(0, a, hash_func);

        //printf("global_sum = %" PRIu64 "ULL global_cnt = %" PRIu64 "ULL\n", global_sum, global_cnt);
    }
    else
    {
        opensieve::sieve(0, 10000000, print_prime_);
    }
    PASS()
    ;
}

/************************************************************************************/
SUITE(general_suite)
{
    RUN_TEST(masking_test);
}

/************************************************************************************/
SUITE(sieve_suite)
{
    RUN_TEST(simple_sieve_test);
    RUN_TEST(complex_sieve_test);
    RUN_TEST(file_sieve_test);
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
    GREATEST_MAIN_END(); /* display results */
}

