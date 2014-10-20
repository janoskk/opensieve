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

#define MASKING_TEST_LENGTH (1<<13)

/************************************************************************************/
TEST up_to_1_billion_test()
{
    opensieve::sieve(0, 1000000000, 0);
    PASS()
    ;
}

/************************************************************************************/
SUITE(up_to_1_billion)
{
    RUN_TEST(up_to_1_billion_test);
}

/************************************************************************************/
TEST up_to_10_billion_test()
{
    opensieve::sieve(0, 10000000000, 0);
    PASS()
    ;
}

/************************************************************************************/
SUITE(up_to_10_billion)
{
    RUN_TEST(up_to_10_billion_test);
}

/************************************************************************************/
TEST c_masking_test()
{
    uint64_t arr[MASKING_TEST_LENGTH];
    for (uint64_t i = 0; i < 100000; i++)
    {
        opensieve::internal::c_masking(arr, MASKING_TEST_LENGTH, i * MASKING_TEST_LENGTH);
    }
    PASS()
    ;
}

/************************************************************************************/
SUITE(c_masking)
{
    RUN_TEST(c_masking_test);
}

/************************************************************************************/
TEST asm_masking_test()
{
    uint64_t arr[MASKING_TEST_LENGTH];
    for (uint64_t i = 0; i < 100000; i++)
    {
        asm_masking(arr, MASKING_TEST_LENGTH, i * MASKING_TEST_LENGTH);
    }
    PASS()
    ;
}

/************************************************************************************/
SUITE(asm_masking)
{
    RUN_TEST(asm_masking_test);
}

/* Add definitions that need to be in the test runner's main file. */
GREATEST_MAIN_DEFS()
;

/************************************************************************************/
int main(int argc, char **argv)
{
    GREATEST_MAIN_BEGIN()
    ; /* command-line arguments, initialization. */
    RUN_SUITE(up_to_1_billion);
    RUN_SUITE(up_to_10_billion);
    RUN_SUITE(c_masking);
    RUN_SUITE(asm_masking);
    GREATEST_MAIN_END(); /* display results */
}

