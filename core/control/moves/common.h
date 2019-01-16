#pragma once

#include <stdint.h>
#include <stdlib.h>

uint32_t feed2delay(double feed, double len, uint32_t steps);

double accelerate(double feed, int32_t acc, uint32_t delay);

uint32_t acceleration_steps(double feed0,
                            double feed1,
                            int32_t acc,
                            double len,
                            uint32_t steps);

