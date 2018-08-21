#pragma once

#include <stdint.h>

#include "steppers.h"

typedef struct {
	int32_t x[3];
	int32_t s[3];
	int32_t maxi;
	int32_t feed;
	int32_t feed0;
	int32_t feed1;
	int32_t len;
	int32_t steps;
	int32_t acc_steps;
	int32_t dec_steps;
	int32_t acceleration;
} line_plan;

void line_init(steppers_definition definition);

void line_pre_calculate(line_plan *line);

int line_step_tick(void);

int line_move_to(line_plan *plan);

