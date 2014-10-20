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

#ifndef OPENSIEVE_H_
#define OPENSIEVE_H_

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

namespace opensieve
{
#define PRINT_PRIME opensieve::print_prime

/**
 * Process function that will be called for each prime after the interval is sieved.
 * Mainly for internal use.
 *
 * prime: current prime number to be processed
 */
typedef void SIEVE_PROCESS_FUNC(uint64_t prime);

/**
 * Standard process function that prints out the prime numbers.
 *
 * prime: current prime number
 */
void print_prime(uint64_t prime);

/**
 * Sieves out the [a*2^k..b*2^k] interval where a*2^k <= first_number < last_number <= b*2^k.
 * The process function will be called for each prime between the first and last numbers.
 *
 * first_number: first number of the inner interval
 * last_number: last number of the inner interval
 * process_for_primes: process function
 */
void sieve(uint64_t first_number, uint64_t last_number, SIEVE_PROCESS_FUNC *process_for_primes);

namespace internal
{
/**
 * Sieves out the table with the primes 3, 5, 7, 11, 13, 17 and 19 with masks.
 *
 * table: sieve table what is a bit table representing the odd numbers
 * length: length of the table
 * table_offset: current table's offset
 */
void c_masking(uint64_t table[], unsigned length, unsigned table_offset);

/**
 * Allocates and sieves out the interval [0..limit] into the table. Note that
 * 1) The caller has to free the allocated memory!
 * 2) The allocated size might be greater than the necessary size [0..2^s] limit <= 2^s
 *
 * limit: upper bound of the sieved numbers
 * table: sieve table what is a bit table representing the odd numbers
 * table_size: size of the sieve table what is allocated by the function
 */

void sieve_small(uint64_t limit, uint64_t **table, uint64_t& table_size);
/**
 * Going through the table and calling the process function for every prime number.
 *
 * process_for_primes: process function
 * table: sieve table
 * table_size: size of the sieve table
 * current_segment: for simple sieve it's always zero, but for segmented sieve it's
 *                  the current number of segment
 * first_number: first number to be processed
 * last_number: last number to be processed
 *
 * returns with the last processed prime
 */
uint64_t process_primes(SIEVE_PROCESS_FUNC *process_for_primes, uint64_t *table, uint64_t table_size,
        uint64_t current_segment = 0, uint64_t first_number = 0, uint64_t last_number = 0);

/**
 * Sieves out no_of_segments segments starting with the first_segment. When the current segment has
 * sieved, the proces function will be called.
 *
 * first_segment: start the sieve with that segment
 * no_of_segments: that many segments will be sieved
 * process_for_primes: process function
 * first_number: first number to be processed
 * last_number: last number to be processed
 */
void sieve_segments(uint64_t first_segment, uint64_t no_of_segments, SIEVE_PROCESS_FUNC *process_for_primes,
        uint64_t *table = 0, uint64_t first_number = 0, uint64_t last_number = 0);
}
}

#endif /* OPENSIEVE_H_ */
