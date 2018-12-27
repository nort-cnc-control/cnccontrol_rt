#pragma once

#include <stdint.h>
#include <fixed.h>

uint64_t isqrt(uint64_t x);
fixed fsqrt(fixed x);

#define PI (3.1415926535)

#define min(a,b) ((a) < (b) ? (a) : (b))
#define max(a,b) ((a) > (b) ? (a) : (b))

#define abs(a) ((a) > 0 ? (a) : (-(a)))
