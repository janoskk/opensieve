opensieve
=========

What it does?
-------------

High-performance implementation of the sieve of Eratosthenes. It is optimized for the x86_64 architecture and tested only with gcc unser OS/X and Linux. Currently, the following is implemented:
- bit-based representation of the even numbers only
- presieved with the first 7 (even) primes with masks
- improved with the wheel sieve method modulo 30
- speeded up with usage of segments 

On a modern computer it sieves out up to 1'000'000'000 and iterates through the primes in less than 1 second (tested with i7-3615QM processor).

What's next?
------------

Further performance improvements might be also applied. 

To contribute?
--------------

Yes, please! Your help is always highly welcome!
