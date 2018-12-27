#include "math.h"

uint64_t isqrt(uint64_t x)
{
    uint64_t r = 1;

    if (x == 0)
        return 0;

    while (abs(x/r - r) > 1)
    {
        r = (x / r + r)/2;
    }
    while (r*r > x)
        r--;
    return r;
}

fixed fsqrt(fixed x)
{
    if (x < 0)
        return -1;
    fixed r = 1;

    if (x == 0)
        return 0;

    while (abs(DIV(x, r) - r) > 1)
    {
        r = (DIV(x, r) + r)/2;
    }

    while (SQR(r) > x)
        r -= 1;
    return r;
}

