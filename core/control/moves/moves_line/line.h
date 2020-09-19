#pragma once

#include <stdint.h>

#include <control/moves/moves_common/common.h>
#include <control/moves/moves_common/steppers.h>

typedef struct {
    // Specified data
    int32_t x[3]; // delta.              steps
    double feed;  // feed of moving.     mm / sec
    double feed0; // initial feed.       mm / sec
    double feed1; // finishing feed.     mm / sec
    double acceleration; // acceleration mm / sec^2
    int (*check_break)(int32_t *dx, void *user_arg);
    void *check_break_data;

    // Pre-calculated data
    double len;            // length of delta. mm
    int maxi;              // which axis has max steps
    uint32_t steps;        // total amount of steps
    uint32_t acc_steps;    // steps on acceleration
    uint32_t dec_steps;    // steps on deceleration
} line_plan;

// pre-calculate parameters of moving
void line_pre_calculate(line_plan *line);

// init line moving
int line_move_to(line_plan *plan);

// tick
int line_step_tick(void);

