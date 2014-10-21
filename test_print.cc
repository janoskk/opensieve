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
