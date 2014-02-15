#ifndef OPENSIEVE_H_
#define OPENSIEVE_H_


/**
 * Process function what will be called for each prime after the current segment is
 * sieved out
 *
 * Parameters:
 *              prime: (each) prime number of the sieved segment
 */
typedef void SIEVE_PROCESS_FUNC(uint64_t prime);


/************************************************************************************/
void open_sieve(uint64_t limit, uint64_t **table, uint64_t& table_size);


/************************************************************************************/
void process_primes(SIEVE_PROCESS_FUNC *process_for_primes,
                    uint64_t *table,
                    uint64_t table_size);

/************************************************************************************/
int old_main(void);


#endif /* OPENSIEVE_H_ */
