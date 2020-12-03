#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include <control/moves/moves_common/steppers.h>

typedef struct {
    uint8_t en:1;
    uint8_t dir:1;
} step_flags;

typedef struct {
    int32_t pos[3];
    int dir[3];
    double speed[3];
    step_flags flags[3];
} cnc_position;

// Math functions
double feed2delay(double feed, double step_len);

void moves_common_init(steppers_definition *definition);
void moves_common_reset(void);

// Movement functions
void moves_common_set_dir(int i, bool dir);
void moves_common_make_step(int i);

void moves_common_line_started(void);
void moves_common_endstops_touched(void);
void moves_common_line_finished(void);

double moves_common_step_len(int8_t dx, int8_t dy, int8_t dz);

// State
void moves_common_set_position(const int32_t *x);

extern cnc_position position;
extern steppers_definition *moves_common_def;

