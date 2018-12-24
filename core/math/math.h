#pragma once

#include <stdint.h>

uint64_t isqrt(uint64_t x);

#define min(a,b) ((a) < (b) ? (a) : (b))
#define max(a,b) ((a) > (b) ? (a) : (b))

#define abs(a) ((a) > 0 ? (a) : (-(a))) 
