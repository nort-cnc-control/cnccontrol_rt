#pragma once

#include <stdint.h>
#include <fixed.h>

#include "steppers.h"

typedef struct {
    // Specified data
    fixed x[3]; // delta
    fixed feed;   // feed of moving
    fixed feed0;  // initial feed
    fixed feed1;  // finishing feed
    uint32_t acceleration; // acceleration
    int (*check_break)(int32_t *dx, void *user_arg);
    void *check_break_data;

    // Pre-calculated data
    fixed len;  // length of delta
    int32_t s[3]; // steps along each axises
    int maxi;     // which axis has max steps
    uint32_t steps;        // total amount of steps
    uint32_t acc_steps;    // steps on acceleration
    uint32_t dec_steps;    // steps on deceleration
} line_plan;

void line_init(steppers_definition definition);

// pre-calculate parameters of moving
void line_pre_calculate(line_plan *line);

// init line moving
int line_move_to(line_plan *plan);

// tick
int line_step_tick(void);
