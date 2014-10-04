opensieve
=========

What it does?
-------------

High-performance implementation of the sieve of Eratosthenes algorithm. It is optimized for the x86_64 architecture and tested only with gcc. Currently, the following is implemented:
- bit-based representation
- only the even numbers are stored
- presieved with the first 7 (even) primes with masks
- wheel sieve method modulo 30 is applied
- (single threaded)

On a modern computer it sieves out up to 1'000'000'000 and iterates through the primes in less than 1 seconds (tested with i7-3615QM processor).

What's next?
------------

To be able to sieve up to 2^64, the segmented version of sieve needs to be implemented as the most important enhancement. In addition, further performance improvements might be also applied. 

To contribute?
--------------

Yes, please! Your help is always highly welcome!
