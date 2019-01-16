#pragma once

#include <stdint.h>

#define fixed_k 1024
typedef int64_t fixed;

#define FIXED_ENCODE(x) (((int64_t)(x)) * fixed_k)
#define FIXED_DECODE(x) ((x) / fixed_k)
#define FIXED_DECODE_ROUND(x) ((x) > 0 ? (((x) + fixed_k/2) / fixed_k) : (((x) - fixed_k/2) / fixed_k))

#define MUL(a, b) ((a)*(b)/fixed_k)

#define DIV(a, b) (((a)*fixed_k) / (b))

#define SQR(a) MUL(a,a)

