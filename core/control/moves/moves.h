#pragma once

#include <stdint.h>
#include <fixed.h>

#include "steppers.h"
#include "line.h"

typedef struct {
    uint8_t en:1;
    uint8_t dir:1;
} step_flags;

typedef struct {
    fixed pos[3];
    fixed speed[3];
    step_flags flags[3];
} cnc_position;

void moves_init(steppers_definition definition);

cnc_endstops moves_get_endstops(void);

int moves_line_to(line_plan *plan);

int moves_step_tick(void);

void moves_set_position(fixed x[3]);

extern cnc_position position;

