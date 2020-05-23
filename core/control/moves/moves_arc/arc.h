#pragma once
#include <common.h>
#include <steppers.h>
#include "err.h"

typedef enum {
    RIGHT = 0,
    TOP,
    LEFT,
    BOTTOM,
} arc_segment_id;

typedef enum {
    XY = 0,
    YZ,
    ZX,
} arc_plane;

typedef struct {
    int32_t x1, y1;
    int32_t x2, y2;
    arc_segment_id segment;
} arc_segment_desc;

typedef struct {
    arc_segment_desc segments[5];
    int num_segments;        // amount of arc segments
    double a2;
    double b2;
} arc_geometry;


typedef struct {
    // Specified data
    arc_plane plane;        // selected plane
    int32_t x[3];           // delta
    double a, b;
    double feed;            // feed of moving
    double feed0;           // initial feed
    double feed1;           // finishing feed
    uint32_t acceleration;  // acceleration

    int (*check_break)(int32_t *dx, void *user_arg);
    void *check_break_data;

    // Pre-calculated data
    double len;            // arc length
    uint32_t steps;        // total amount of steps
    uint32_t acc_steps;    // steps on acceleration
    uint32_t dec_steps;    // steps on deceleration

    arc_geometry geometry;

    // flags
    struct {
        int cw : 1;      // True if clock-wise
        int ready : 1;   // Plan is calculated
    };
} arc_plan;

void arc_init ( steppers_definition definition );

void arc_pre_calculate ( arc_plan *arc );

int arc_move_to(arc_plan *plan);

int arc_step_tick(void);

