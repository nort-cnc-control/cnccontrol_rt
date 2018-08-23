#pragma once

#include <stdint.h>

#include "steppers.h"
#include "line.h"

typedef struct {
	uint8_t en:1;
	uint8_t dir:1;
} step_flags;

typedef struct {
	int32_t pos[3];
	int32_t speed[3];
	step_flags flags[3];
} cnc_position;

void moves_init(steppers_definition definition);

cnc_endstops moves_get_endstops(void);

int moves_line_to(line_plan *plan);

int moves_step_tick(void);

void moves_set_position(int32_t x[3]);

extern cnc_position position;

