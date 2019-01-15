#pragma once

#include <fixed.h>
#include "steppers.h"
#include "err.h"

typedef struct {
    int32_t start[2];
    int32_t finish[2];
    int32_t a;
    int32_t b;
    struct {
        int go_x:1;
        int go_y:1;
        int sign:2;
    };
    int stx;
    int sty;
} arc_segment;

typedef enum {
    XY = 0,
    YZ = 1,
    ZX = 2,
} arc_plane;

typedef struct {
    // Specified data
    arc_plane plane;       // selected plane
    fixed x[3];            // delta
    fixed d;               // center in selected plane
    fixed feed;            // feed of moving
    fixed feed0;           // initial feed
    fixed feed1;           // finishing feed
    uint32_t acceleration; // acceleration
    int (*check_break)(int32_t *dx, void *user_arg);
    void *check_break_data;

    // Pre-calculated data
    uint32_t steps;        // total amount of steps
    uint32_t acc_steps;    // steps on acceleration
    uint32_t dec_steps;    // steps on deceleration

    arc_segment segments[5]; // arc segments
    int num_segments;        // amount of arc segments

    // flags
    struct {
        int cw : 1;      // True if clock-wise
        int ready : 1;   // Plan is calculated
    };
} arc_plan;

void arc_pre_calculate ( arc_plan *arc );

int arc_move_to(arc_plan *plan);

int arc_step_tick(void);

