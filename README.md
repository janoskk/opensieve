opensieve
=========

What it does?
-------------

High-performance implementation of the sieve of Eratosthenes. It is optimized for the x86_64 architecture and tested only with gcc 4.9 and clang 3.4 under OS X and Linux. Currently, the following is implemented:

- bit-based representation of the even numbers only
- presieved with the first 7 (even) primes with masks
- improved with the wheel sieve method modulo 30
- speeded up with usage of segments 


How to use?
-----------

Have a look on the following code to print out the primes in the given period. This code can be also found in test_print.cc.
```c++
#include <stdio.h>
#include <stdlib.h>
#include "opensieve.h"

int main(int argc, char **argv)
{
    switch (argc)
    {
    case 2:
        opensieve::sieve(0, atoll(argv[1]), PRINT_PRIME);
        break;
    case 3:
        opensieve::sieve(atoll(argv[1]), atoll(argv[2]), PRINT_PRIME);
        break;
    default:
        printf("Sieve out the given interval.\n"
                "Usage: %s [start] end\n"
                "  start:\tfirst number of the interval (default: 0)\n"
                "  end:\tlast number of the interval\n", argv[0]);
        break;
    }
    return 0;
}
```
To get it run, type the following commands after you cloned the repository:
```bash
$ make
$ ./test_print 1 100
```

How fast it is?
---------------
On a modern computer it sieves out up to 1'000'000'000 and iterates through the primes in less than 1 second (tested with i7-3615QM processor).


To contribute?
--------------

Yes, please! Your help is always highly welcome! Open tasks:

- Support for Widows
