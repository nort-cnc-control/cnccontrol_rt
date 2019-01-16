#pragma once

#include <stdint.h>

#define fixed_k (1UL << 22)
typedef __int128 fixed;

#define FIXED_ENCODE(x) (((fixed)(x)) * fixed_k)
#define FIXED_DECODE(x) ((x) / fixed_k)
#define FIXED_DECODE_ROUND(x) ((x) > 0 ? (((x) + fixed_k/2) / fixed_k) : (((x) - fixed_k/2) / fixed_k))

#define MUL(a, b) ((a)*(b)/fixed_k)

#define DIV(a, b) (((a)*fixed_k) / (b))

#define SQR(a) MUL(a,a)

