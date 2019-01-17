#pragma once

#include <stdint.h>
#include <stdlib.h>

uint32_t feed2delay(double feed, double step_len);

double accelerate(double feed, double acc, double delay);

uint32_t acceleration_steps(double feed0,
                            double feed1,
                            double acc,
                            double len,
                            uint32_t steps);
