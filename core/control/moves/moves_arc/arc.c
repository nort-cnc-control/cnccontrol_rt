#define __STDC_WANT_DEC_FP__
#include <math.h>
#include <stdlib.h>
#include <assert.h>
//#include <stdio.h>

#include <control/moves/moves_common/common.h>
#include <control/moves/moves_common/acceleration.h>
#include <control/moves/moves_arc/arc.h>

#define SQR(a) ((a) * (a))
const double pi = 3.1415926535;

#define COS_RESYNC_FRQ 8

static steppers_definition def;
void arc_init(steppers_definition definition)
{
    def = definition;
}


// Running

static arc_plan *current_plan;

static struct
{
    double t;
    double tv;
    double cost;
    double sint;

    struct {
        int32_t x;
        int32_t y;
        int32_t z;
    } plane;

    struct {
        int32_t position[3];
    } global;

    int32_t position[3];
    int32_t dir[3];
    acceleration_state acc;
} current_state;

static void global_update_position(void)
{
    switch (current_plan->plane)
    {
        case XY: // XY
            current_state.global.position[0] = current_state.plane.x;
            current_state.global.position[1] = current_state.plane.y;
            current_state.global.position[2] = current_state.plane.z;
            break;

        case YZ: // YZ
            current_state.global.position[0] = current_state.plane.z;
            current_state.global.position[1] = current_state.plane.x;
            current_state.global.position[2] = current_state.plane.y;
            break;

        case ZX: // ZX
            current_state.global.position[0] = current_state.plane.y;
            current_state.global.position[1] = current_state.plane.z;
            current_state.global.position[2] = current_state.plane.x;
            break;
    }
}

static bool iterate(arc_plan *plan)
{
    double dxdt = -plan->a * current_state.sint;
    double dydt = plan->b * current_state.cost;
    double dzdt = plan->h;

//    printf("-----\n");

    double dtx;
    if (dxdt != 0)
        dtx = 0.5/fabs(dxdt);
    else
        dtx = INFINITY;

    double dty;
    if (dydt != 0)
        dty = 0.5/fabs(dydt);
    else
        dty = INFINITY;

    double dtz;
    if (dzdt != 0)
        dtz = 0.5/fabs(dzdt);
    else
        dtz = INFINITY;

    double dt = fmin(fmin(fmin(dtx, dty), dtz), fabs(current_state.t - plan->t_end));
    if (dt <= 0)
        return false;

    if (current_plan->cw)
        dt = -dt;

    current_state.t += dt;
//    printf("%lf\n", current_state.t / pi);
    current_state.tv += dt;
    if (current_state.tv >= pi/COS_RESYNC_FRQ)
    {
//        printf("*** resync\n");
        current_state.tv -= pi/COS_RESYNC_FRQ;
        current_state.cost = cos(current_state.t);
        current_state.sint = sin(current_state.t);
    }
    else if (current_state.tv < 0)
    {
//        printf("*** resync\n");
        current_state.tv += pi/COS_RESYNC_FRQ;
        current_state.cost = cos(current_state.t);
        current_state.sint = sin(current_state.t);
    }
    else
    {
        double cost = current_state.cost - dt * current_state.sint - dt*dt/2*current_state.cost;
        double sint = current_state.sint + dt * current_state.cost - dt*dt/2*current_state.sint;
    
        current_state.cost = cost;
        current_state.sint = sint;
    }

    current_state.plane.x = round(plan->a * current_state.cost);
    current_state.plane.y = round(plan->b * current_state.sint);
    current_state.plane.z = round(plan->h * (current_state.t - current_plan->t_start));
//    printf("cos = %lf sin = %lf\n", current_state.cost, current_state.sint);
//    printf("CS: %i %i %i\n", current_state.plane.x, current_state.plane.y, current_state.plane.z);
    return true;
}

static bool make_tick(void)
{
    int i;
    int32_t pos[3];
    for (i = 0; i < 3; i++)
        pos[i] = current_state.global.position[i];

    while (true)
    {
        if (!iterate(current_plan))
            return false;
        global_update_position();

        // make steps
        bool has_step = false;
        for (i = 0; i < 3; i++)
        {
            int d = current_state.global.position[i] - pos[i];
            if (d > 1 || d < -1)
            {
//                printf("*** 2 steps at once\n");
                  // TODO: Raise error
            }
            if (d != 0)
            {
                moves_common_set_dir(i, d >= 0);
                moves_common_make_step(i);
                has_step = true;
            }

            current_state.dir[i] = d;
            current_state.position[i] += d;
        }

        if (has_step)
            break;
    }

    return true;
}


// module API functions

int32_t arc_step_tick(void)
{
    // Check for endstops
    if (current_plan->check_break && current_plan->check_break(current_state.dir, current_plan->check_break_data))
    {
        moves_common_endstops_touched();
        return -E_ENDSTOP;
    }

    // Make step
    if (!make_tick())
    {
        moves_common_line_finished();
        return -E_NEXT;
    }

    // Calculate delay
    double step_len = current_plan->len / current_plan->steps;
    double step_delay = step_len / current_state.acc.feed;
//    printf("%lf\n", current_state.acc.feed);
    acceleration_process(&current_state.acc, step_delay);
    return step_delay * 1000000L;
}

static void arc_init_move(arc_plan *plan)
{
    current_state.t = plan->t_start;
    current_state.cost = cos(current_state.t);
    current_state.sint = sin(current_state.t);

    current_state.tv = plan->t_start;
    while (current_state.tv >= pi/COS_RESYNC_FRQ)
        current_state.tv -= pi/COS_RESYNC_FRQ;
    while (current_state.tv < 0)
        current_state.tv += pi/COS_RESYNC_FRQ;

    current_state.plane.x = plan->a * current_state.cost;
    current_state.plane.y = plan->b * current_state.sint;
    current_state.plane.z = 0;

    global_update_position();

//    printf("=================\nstart %lf %lf\n", plan->t_start, plan->t_end);
}

int arc_move_to(arc_plan *plan)
{
    current_plan = plan;
    if (!plan->ready)
    {
        arc_pre_calculate(plan);
    }

    current_state.acc.acceleration = current_plan->acceleration;
    current_state.acc.feed = current_plan->feed0;
    current_state.acc.target_feed = current_plan->feed;
    current_state.acc.end_feed = current_plan->feed1;
    current_state.acc.type = STATE_ACC;
    current_state.acc.step = 0;
    current_state.acc.total_steps = current_plan->steps;
    current_state.acc.acc_steps = current_plan->acc_steps;
    current_state.acc.dec_steps = current_plan->dec_steps;
 
    arc_init_move(plan);

    moves_common_line_started();
    return -E_OK;
}

void arc_pre_calculate(arc_plan *arc)
{
    double stpu_x;
    double stpu_y;
    double stpu_z;
    
    double x1, y1;
    double x2, y2;
    double H;

    x1 = arc->x1[0];
    y1 = arc->x1[1];
    x2 = arc->x2[0];
    y2 = arc->x2[1];
    H  = arc->H;

    switch (arc->plane)
    {
    case XY:
        stpu_x = moves_common_def->steps_per_unit[0];
        stpu_y = moves_common_def->steps_per_unit[1];
        stpu_z = moves_common_def->steps_per_unit[2];
        break;
    case YZ:
        stpu_x = moves_common_def->steps_per_unit[1];
        stpu_y = moves_common_def->steps_per_unit[2];
        stpu_z = moves_common_def->steps_per_unit[0];
        break;
    case ZX:
        stpu_x = moves_common_def->steps_per_unit[2];
        stpu_y = moves_common_def->steps_per_unit[0];
        stpu_z = moves_common_def->steps_per_unit[1];
        break;
    }

    arc->t_start = atan2(y1, x1);
    arc->t_end = atan2(y2, x2);
    if (arc->cw)
    {
        while (arc->t_end > arc->t_start)
            arc->t_end -= 2*pi;
    }
    else
    {
        while (arc->t_end < arc->t_start)
            arc->t_end += 2*pi;
    }

    arc->h = H / (arc->t_end - arc->t_start);
    arc->cost_start = cos(arc->t_start);
    arc->sint_start = sin(arc->t_start);

    /* check feeds */
    if (arc->feed < moves_common_def->feed_base)
        arc->feed = moves_common_def->feed_base;
    else if (moves_common_def->feed_max > 0 && arc->feed > moves_common_def->feed_max)
        arc->feed = moves_common_def->feed_max;

    if (arc->feed1 < moves_common_def->feed_base)
        arc->feed1 = moves_common_def->feed_base;
    else if (arc->feed1 > arc->feed)
        arc->feed1 = arc->feed;

    if (arc->feed0 < moves_common_def->feed_base)
        arc->feed0 = moves_common_def->feed_base;
    else if (arc->feed0 > arc->feed)
        arc->feed0 = arc->feed;

    /* calculate total steps */
    arc_init_move(arc);
    arc->steps = 0;
    while (iterate(arc))
    {
        arc->steps++;
    }

    /* calculate acceleration and deceleration steps */
    arc->acc_steps = acceleration_steps(arc->feed0, arc->feed, arc->acceleration, arc->len / arc->steps);
    arc->dec_steps = acceleration_steps(arc->feed1, arc->feed, arc->acceleration, arc->len / arc->steps);

    arc->ready = 1;
}

