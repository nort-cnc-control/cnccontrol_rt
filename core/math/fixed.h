#pragma once

#include <stdint.h>

static const int fixed_k = 1000;
typedef int64_t fixed;

#define FIXED_ENCODE(x) (((int64_t)(x)) * fixed_k)
#define FIXED_DECODE(x) ((x) / fixed_k)

#define MUL(x, y) (((x)*(y))/fixed_k)
#define DIV(x, y) (((x)*fixed_k)/(y))
#define SQR(x) SQR(x, x)

