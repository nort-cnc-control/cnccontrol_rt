#include "math.h"

#define abs(x) ((x) >= 0 ? (x) : (-(x)))

uint32_t isqrt(uint32_t x)
{
	int r = 1;

	if (x == 0)
		return 0;

	while (abs(x/r - r) > 1)
	{
		r = (x / r + r + 1)/2;
	}
	return r;
}

