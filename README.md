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

Notes on the implementation
---------------------------
- Choosing the segment size is not obvious, therefore I did some tests to have an impression. I sieved out up to 10 billion with different segment sizes. The following table contains the summary of the results where segment size means the number of represented integers in the given segment, the running time means CPU time in second.

segment size | running time | segment size | running time
-------------|--------------|--------------|-------------
2^10|2040.628|**2^21**|**10.388**
2^11|1045.914|2^22|10.459
2^12|526.396|2^23|11.615
2^13|269.197|2^24|12.121
2^14|140.041|2^25|12.643
2^15|74.997|2^26|16.504
2^16|42.527|2^27|25.195
2^17|25.918|2^28|31.656
2^18|17.229|2^29|35.086
2^19|12.561|2^30|40.192
2^20|10.871|2^31|50.124



To contribute?
--------------

Yes, please! Your help is always highly welcome! Open tasks:

- Support for Widows
