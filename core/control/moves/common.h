#pragma once

#include <fixed.h>

uint32_t feed2delay(fixed feed, fixed len, uint32_t steps);

fixed accelerate(fixed feed, uint32_t acc, uint32_t delay);

uint32_t acceleration_steps(fixed feed0,
                            fixed feed1,
                            int32_t acc,
                            fixed len,
                            uint32_t steps);
