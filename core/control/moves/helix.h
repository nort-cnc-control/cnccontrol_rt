#pragma once

#include <fixed.h>

typedef struct {
    // Specified data
    enum {
        XY = 0,
        YZ = 1,
        ZX = 2,
    } plane;         // selected plane
    fixed x[3];      // delta
    fixed d;         // center in selected plane
    fixed feed;   // feed of moving
    fixed feed0;  // initial feed
    fixed feed1;  // finishing feed
    uint32_t acceleration; // acceleration
    int (*check_break)(int32_t *dx, void *user_arg);
    void *check_break_data;

    // Pre-calculated data
    fixed radius;          // radius
    fixed angle;           // angle of arc
    fixed pld[2];          // delta in selected plane
    fixed center[2];       // center of arc
    fixed height;          // delta in 3-rd axis
    fixed length;          // length of arc in plane
    uint32_t steps;        // total amount of steps
    uint32_t acc_steps;    // steps on acceleration
    uint32_t dec_steps;    // steps on deceleration

    // flags
    struct {
        int big_arc : 1; // True if > 180 degrees
        int cw : 1;      // True if clock-wise
    };
} helix_plan;
