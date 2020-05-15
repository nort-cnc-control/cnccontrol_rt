#pragma once

#include <stdint.h>

#include "steppers.h"
#include "line.h"
#include "arc.h"

typedef struct {
    uint8_t en:1;
    uint8_t dir:1;
} step_flags;

typedef struct {
    _Decimal64 pos[3];
    _Decimal64 speed[3];
    step_flags flags[3];
} cnc_position;

void moves_init(steppers_definition definition);

cnc_endstops moves_get_endstops(void);

int moves_line_to(line_plan *plan);

int moves_arc_to(arc_plan *plan);

int moves_step_tick(void);

void moves_reset(void);

void moves_break(void);

void moves_set_position(_Decimal64 x[3]);

extern cnc_position position;

