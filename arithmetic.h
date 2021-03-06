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

#ifndef ARITHMETIC_H_
#define ARITHMETIC_H_

/************************************************************************************/
/**
 * Shift the mask to right
 */
#define MASK_R(acc, c, num, num_var, count)   \
{                                             \
        num_var >>= count;                    \
        c = num_var;                          \
        c <<= num;                            \
        num_var |= c;                         \
        acc |= num_var;                       \
}

/************************************************************************************/
/**
 * Shift the mask to left
 */
#define MASK_L(acc, c, num, num_var, count)   \
{                                             \
        num_var <<= count;                    \
        c = num_var;                          \
        c >>= num;                            \
        num_var |= c;                         \
        acc |= num_var;                       \
}

/************************************************************************************/
/**
 * Assembly function for the masking
 */
extern "C"
{
uint64_t asm_masking(uint64_t *ptr, unsigned length, unsigned table_offset);
}

/**
 * Assembly function for testing during the development
 */
extern "C"
{
uint64_t testing(uint64_t *ptr, unsigned length);
}

/************************************************************************************/
/**
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
/**
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

#endif
