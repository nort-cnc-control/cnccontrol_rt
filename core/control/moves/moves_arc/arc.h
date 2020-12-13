#pragma once

#include <control/moves/moves_common/common.h>
#include <control/moves/moves_common/steppers.h>
#include <err/err.h>

typedef enum {
    XY = 0,
    YZ,
    ZX,
} arc_plane;

typedef struct {
    // Specified data
    arc_plane plane;        // selected plane
    double x1[2];           // start point in local crds
    double x2[2];           // end point in local crds
    double H;               // height of helix
    double a, b;
    double feed;            // feed of moving
    double feed0;           // initial feed
    double feed1;           // finishing feed
    uint32_t acceleration;  // acceleration

    int (*check_break)(int32_t *dx, void *user_arg);
    void *check_break_data;

    // Pre-calculated data
    double len;            // arc length
    double t_acc;    // steps on acceleration
    double t_dec;    // steps on deceleration
    double t_start, t_end;
    double cost_start, sint_start;
    double h;

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
double arc_movement_feed(void);
double arc_acceleration_process(double len);
bool arc_check_endstops(void);

