#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <vector>

#define GREATEST_STDOUT stderr
#include "greatest.h"
#include "opensieve.h"

#define PATTERN_SEGMENT_SIZE 1024

uint64_t global_sum = 0;
uint64_t global_cnt = 0;


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
            printf("\t %llx\n", hex);
            hex = 0;
        }

    }
    printf("\n");
}


/************************************************************************************/
void old_sieve(uint64_t limit)
{
    uint64_t sqrt_limit = sqrt((double)limit) + 2;

    // generate small primes <= sqrt < 2^32
    char *small_primes = (char*)valloc((limit-1)/2 * sizeof(char));
    if (small_primes == 0)
    {
        fprintf(stderr, "Unable to allocate small_primes!\n");
        return;
    }

    memset(small_primes, 0xffffffff, ((limit-1)/2) * sizeof(char));
    for (uint64_t i = 3; i <= sqrt_limit; i+=2)
    {
        if (small_primes[(i-1)/2])
        {
            for (int64_t j = i; i * j <= limit; j += 2)
            {
                small_primes[(i * j - 1) / 2] = 0;
            }
        }
    }

    // calculate the exact number of primes for allocation
    uint64_t counter = 0;
    for (uint64_t i = 1; i < (limit-1)/2; i++)
    {
        if (small_primes[i])
        {
            // printf("B %llu\n", i*2+1);
            counter++;
        }
    }
    printf("primes: %llu\n", counter);
}



/************************************************************************************/
/*static void print_prime(int64_t prime)
{
    printf("%lld\n", prime);
}
*/


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
        { { 1000000, 37550402023, 78498 },
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

        open_sieve(hash_results[i][0], &table, table_size);
        process_primes(hash_func, table, table_size);

        free(table);

        //printf("global_sum = %llu global_cnt = %llu\n", global_sum, global_cnt);

        ASSERT_EQ(global_sum, hash_results[i][1]);
        ASSERT_EQ(global_cnt, hash_results[i][2]);
    }

    PASS();
}


/************************************************************************************/
SUITE(sieve_suite)
{
    RUN_TEST(simple_sieve_test);
}


/************************************************************************************/
int devel_tests(void)
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


/************************************************************************************/
TEST devel_tests_case()
{
    devel_tests();
    PASS();
}


/************************************************************************************/
SUITE(devel_tests_suite)
{
    RUN_TEST(devel_tests_case);
}


/* Add definitions that need to be in the test runner's main file. */
GREATEST_MAIN_DEFS();


/************************************************************************************/
int main(int argc, char **argv)
{
    GREATEST_MAIN_BEGIN();      /* command-line arguments, initialization. */
    RUN_SUITE(sieve_suite);
    RUN_SUITE(devel_tests_suite);
    GREATEST_MAIN_END();        /* display results */
}


