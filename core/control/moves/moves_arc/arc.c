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

enum arc_move_state_e
{
    AMS_ARC = 0,
    AMS_FIX,
    AMS_COMPLETE,
    AMS_END,
};

static struct arc_state_s
{
    double t;
    double tv;
    double cost;
    double sint;

    struct {
        int32_t x;
        int32_t y;
        int32_t z;
    } plane_current;

    struct {
        int32_t x;
        int32_t y;
        int32_t z;
    } plane_target;

    struct {
        int32_t position[3];
    } global_current;

    struct {
        int32_t position[3];
    } global_target;

    int8_t steps[3];

    enum arc_move_state_e move_state;

    int32_t dir[3];
    acceleration_state acc;
} current_state;

static void global_update_position(struct arc_state_s *state)
{
    switch (current_plan->plane)
    {
        case XY: // XY
            state->global_current.position[0] = state->plane_current.x;
            state->global_current.position[1] = state->plane_current.y;
            state->global_current.position[2] = state->plane_current.z;

            state->global_target.position[0]  = state->plane_target.x;
            state->global_target.position[1]  = state->plane_target.y;
            state->global_target.position[2]  = state->plane_target.z;
            break;

        case YZ: // YZ
            state->global_current.position[0] = state->plane_current.z;
            state->global_current.position[1] = state->plane_current.x;
            state->global_current.position[2] = state->plane_current.y;

            state->global_target.position[0]  = state->plane_target.z;
            state->global_target.position[1]  = state->plane_target.x;
            state->global_target.position[2]  = state->plane_target.y; 
            break;

        case ZX: // ZX
            state->global_current.position[0] = state->plane_current.y;
            state->global_current.position[1] = state->plane_current.z;
            state->global_current.position[2] = state->plane_current.x;

            state->global_target.position[0]  = state->plane_target.y;
            state->global_target.position[1]  = state->plane_target.z;
            state->global_target.position[2]  = state->plane_target.x; 
            break;
    }
}

static bool iterate_normal(arc_plan *plan, struct arc_state_s *state)
{
    float dxdt = -plan->a * state->sint;
    float dydt = plan->b * state->cost;
    float dzdt = plan->h;

    float maxddt = fmax(fmax(fabs(dxdt), fabs(dydt)), fabs(dzdt));
    float dt = fmin(1/maxddt, fabs(state->t - plan->t_end));

    if (plan->t_end > plan->t_start && state->t > plan->t_end - 1e-6)
        return false;
    if (plan->t_end < plan->t_start && state->t < plan->t_end + 1e-6)
        return false;

    if (plan->cw)
        dt = -dt;

    state->t += dt;
    state->tv += dt;
    if (state->tv >= pi/COS_RESYNC_FRQ)
    {
        state->tv -= pi/COS_RESYNC_FRQ;
        state->cost = cos(state->t);
        state->sint = sin(state->t);
    }
    else if (current_state.tv < 0)
    {
        state->tv += pi/COS_RESYNC_FRQ;
        state->cost = cos(state->t);
        state->sint = sin(state->t);
    }
    else
    {
        double cost = state->cost - dt * state->sint - dt*dt/2*state->cost;
        double sint = state->sint + dt * state->cost - dt*dt/2*state->sint;
    
        state->cost = cost;
        state->sint = sint;
    }

    state->plane_target.x = round(plan->a * state->cost);
    state->plane_target.y = round(plan->b * state->sint);
    state->plane_target.z = round(plan->h * (state->t - plan->t_start));
    return true;
}

static int clip(int x)
{
    if (x < 0)
        return -1;
    if (x == 0)
        return 0;
    return 1;
}

static bool current_move_ready(struct arc_state_s *state)
{
    int i;
    for (i = 0; i < 3; i++)
    {
        if (state->global_current.position[i] != state->global_target.position[i])
            return false;
    }
    return true;
}

static bool iterate(arc_plan *plan, struct arc_state_s *state)
{
    int i;
    if (state->plane_target.x == plan->x2[0] &&
        state->plane_target.y == plan->x2[1] &&
        state->plane_target.z == plan->H)
    {
        state->move_state = AMS_END;
        return false;
    }

    bool arc_res = false;
    switch (state->move_state)
    {
        case AMS_ARC:
            arc_res = iterate_normal(plan, state);
            global_update_position(state);
            break;
        case AMS_FIX:
            break;
        case AMS_COMPLETE:
            break;
        case AMS_END:
//            printf("End: %i %i : %i %i : %i %i\n", (int)plan->x2[0], (int)plan->x2[1], (int)state->plane_current.x, (int)state->plane_current.y, (int)state->plane_target.x, (int)state->plane_target.y);
            return false;
    }

    state->plane_current.x += clip(state->plane_target.x - state->plane_current.x);
    state->plane_current.y += clip(state->plane_target.y - state->plane_current.y);
    state->plane_current.z += clip(state->plane_target.z - state->plane_current.z);
    for (i = 0; i < 3; i++)
    {
        state->steps[i] = clip(state->global_target.position[i] - state->global_current.position[i]);
        state->global_current.position[i] += state->steps[i];
    }

    switch (state->move_state)
    {
        case AMS_ARC:
            if (arc_res == false)
            {
                state->plane_target.x = plan->x2[0];
                state->plane_target.y = plan->x2[1];
                state->plane_target.z = plan->H;
                global_update_position(state);
//                printf("ARC -> COMPLETE\n");
                state->move_state = AMS_COMPLETE;
            }
            else
            {
                if (!current_move_ready(state))
                {
//                    printf("ARC -> FIX\n");
                    state->move_state = AMS_FIX;
                }
            }
            break;
        case AMS_FIX:
            if (current_move_ready(state))
            {
//                printf("FIX -> ARC\n");
                state->move_state = AMS_ARC;
            }
            break;
        case AMS_COMPLETE:
            if (current_move_ready(state))
            {
//                printf("COMPLETE -> END\n");
                state->move_state = AMS_END;
                return false;
            }
            break;
        case AMS_END:
            return false;
    }
    return true;
}

static bool make_tick(int *dx, int *dy, int *dz)
{
    int i;
    if (!iterate(current_plan, &current_state))
    	return false;

    // make steps
    for (i = 0; i < 3; i++)
    {
        int d = current_state.steps[i];
        moves_common_set_dir(i, d >= 0);
        current_state.dir[i] = d;
        if (d != 0)
        {
            moves_common_make_step(i);
        }
    }
    *dx = current_state.steps[0];
    *dy = current_state.steps[1];
    *dz = current_state.steps[2];

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

    int dx, dy, dz;
    // Make step
    if (!make_tick(&dx, &dy, &dz))
    {
        moves_common_line_finished();
        return -E_NEXT;
    }

    // Calculate delay
    double step_len = moves_common_step_len(dx, dy, dz);
    double step_delay = step_len / current_state.acc.feed;
    acceleration_process(&current_state.acc, step_delay, current_state.t);
    return step_delay * 1000000L;
}

static void arc_init_move(arc_plan *plan, struct arc_state_s *state)
{
    state->t    = plan->t_start;
    state->cost = cos(state->t);
    state->sint = sin(state->t);

    state->tv = plan->t_start;
    while (state->tv >= pi/COS_RESYNC_FRQ)
        state->tv -= pi/COS_RESYNC_FRQ;
    while (state->tv < 0)
        state->tv += pi/COS_RESYNC_FRQ;

    state->plane_current.x = plan->x1[0];
    state->plane_current.y = plan->x1[1];
    state->plane_current.z = 0;

    state->plane_target.x = plan->x1[0];
    state->plane_target.y = plan->x1[1];
    state->plane_target.z = 0;

    global_update_position(state);
    current_state.move_state = AMS_ARC;
//    printf("=================\nstart %lf %lf\n", plan->t_start, plan->t_end);
//    printf("Start: %i %i : %i %i : %i %i\n", (int)plan->x2[0], (int)plan->x2[1], (int)state->plane_current.x, (int)state->plane_current.y, (int)state->plane_target.x, (int)state->plane_target.y);
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

    current_state.acc.current_t = current_plan->t_start;
    current_state.acc.start_t   = current_plan->t_start;
    current_state.acc.end_t     = current_plan->t_end;
    current_state.acc.acc_t     = current_plan->t_acc;
    current_state.acc.dec_t     = current_plan->t_dec;
 
    arc_init_move(plan, &current_state);

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

    /* calculate acceleration and deceleration */
    arc->t_acc = acceleration(arc->feed0, arc->feed, arc->acceleration, arc->len, arc->t_start, arc->t_end);
    arc->t_dec = acceleration(arc->feed1, arc->feed, arc->acceleration, arc->len, arc->t_end, arc->t_start);

    arc->ready = 1;
}

