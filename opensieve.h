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

namespace opensieve
{
/**
 * Process function that will be called for each prime after the interval is sieved.
 *
 * prime: current prime number to be processed
 */
typedef void SIEVE_PROCESS_FUNC(uint64_t prime);

/**
 * Sieves out the interval [0..limit] into the allocated table.
 * The caller has to free the allocated memory!
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
 * returns with the last processed prime
 */
uint64_t process_primes(SIEVE_PROCESS_FUNC *process_for_primes, uint64_t *table, uint64_t table_size,
        unsigned current_segment = 0);

/**
 * Sieves out no_of_segments segments starting with the first_segment.
 *
 * first_segment: start the sieve with that segment
 * no_of_segments: that many segments will be sieved
 * process_for_primes: process function
 */
void sieve(int64_t first_segment, int no_of_segments, SIEVE_PROCESS_FUNC *process_for_primes);

/**
 * TODO: docu
 */
void bitsieve(uint64_t table[], unsigned length, unsigned table_offset);

}

#endif /* OPENSIEVE_H_ */
