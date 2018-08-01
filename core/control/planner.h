#pragma once

#include <stdint.h>
#include "moves.h"


void init_planner(steppers_definition pd);

int planner_line_to(int32_t x[3], int feed);

void planner_find_begin(int rx, int ry, int rz);

