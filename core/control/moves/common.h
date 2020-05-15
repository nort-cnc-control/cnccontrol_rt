#pragma once

#include <stdint.h>
#include <stdlib.h>

uint32_t feed2delay(_Decimal64 feed, _Decimal64 step_len);

_Decimal64 accelerate(_Decimal64 feed, _Decimal64 acc, _Decimal64 delay);

uint32_t acceleration_steps(_Decimal64 feed0,
                            _Decimal64 feed1,
                            _Decimal64 acc,
                            _Decimal64 len,
                            uint32_t steps);
