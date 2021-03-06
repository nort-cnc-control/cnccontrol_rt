#include <math.h>

#include <control/moves/moves_common/common.h>
#include <control/moves/moves_line/line.h>
#include <err/err.h>

#include <control/moves/moves_common/acceleration.h>

#define SQR(x) ((x) * (x))

static line_plan *current_plan;

static struct
{
    int8_t dir[3];
    int32_t steps[3];
    int32_t err[3];
    int is_moving;

    acceleration_state acc;

    int32_t start_pos[3];
} current_state;

int line_move_to(line_plan *plan)
{
    int i;
    current_plan = plan;

    if (current_plan->len < 0)
    {
        line_pre_calculate(current_plan);
    }

    for (i = 0; i < 3; i++)
    {
        if (current_plan->x[i] > 0) 
            current_state.dir[i] = 1;
        else if (current_plan->x[i] < 0)
            current_state.dir[i] = -1;
        else 
            current_state.dir[i] = 0;
        current_state.steps[i] = 0;
    }

    if (current_plan->steps == 0)
        return -E_NEXT;

    current_state.acc.acceleration = current_plan->acceleration;
    current_state.acc.feed = current_plan->feed0;
    current_state.acc.target_feed = current_plan->feed;
    current_state.acc.end_feed = current_plan->feed1;
    current_state.acc.type = STATE_ACC;

    current_state.acc.current_t = 0;
    current_state.acc.start_t   = 0;
    current_state.acc.end_t     = current_plan->steps;
    current_state.acc.acc_t     = current_plan->acc_steps;
    current_state.acc.dec_t     = current_plan->dec_steps;
    current_state.is_moving = 1;
    for (i = 0; i < 3; i++)
    {
        current_state.err[i] = 0;
        current_state.start_pos[i] = position.pos[i];
    }
    
    moves_common_line_started();
    return -E_OK;
}

static bool make_step(void)
{
    int i;
    if (current_state.acc.current_t >= current_state.acc.end_t)
    {
        return false;
    }

    /* Bresenham */
    moves_common_schedule_step(current_plan->maxi, current_state.dir[current_plan->maxi]);
    
    for (i = 0; i < 3; i++)
    {
        if (i == current_plan->maxi)
            continue;
        current_state.err[i] += abs(current_plan->x[i]);
        if (current_state.err[i] * 2 >= (int32_t)current_plan->steps)
        {
            current_state.err[i] -= (int32_t)current_plan->steps;
            moves_common_schedule_step(i, current_state.dir[i]);
        }
    }
    return true;
}

bool line_check_endstops(void)
{
    // Check for endstops
    if (current_plan->check_break && current_plan->check_break(current_plan->x, current_plan->check_break_data))
    {
        return true;
    }
    return false;
}

int line_step_tick(void)
{
    // Make step
    if (!make_step())
    {
        return -E_NEXT;
    }

    /* Calculating delay */
    current_state.acc.current_t++;
    return -E_OK;
}

double line_movement_feed(void)
{
    return current_state.acc.feed;
}

double line_acceleration_process(double len)
{
    double dt = len / current_state.acc.feed;
    acceleration_process(&current_state.acc, dt, current_state.acc.current_t);
    return dt;
}

static void bresenham_plan(line_plan *plan)
{
    int i;
    plan->steps = abs(plan->x[0]);
    plan->maxi = 0;
    if (abs(plan->x[1]) > plan->steps)
    {
        plan->maxi = 1;
        plan->steps = abs(plan->x[1]);
    }

    if (abs(plan->x[2]) > plan->steps)
    {
        plan->maxi = 2;
        plan->steps = abs(plan->x[2]);
    }
}

void line_pre_calculate(line_plan *line)
{
    int j;
    double l = 0;
    for (j = 0; j < 3; j++)
    {
        double d = line->x[j] / moves_common_def.steps_per_unit[j];
        l += d*d;
    }
    line->len = sqrt(l);

    if (line->len == 0)
        return;

    if (line->feed < moves_common_def.feed_base)
        line->feed = moves_common_def.feed_base;
    else if (moves_common_def.feed_max > 0 && line->feed > moves_common_def.feed_max)
        line->feed = moves_common_def.feed_max;

    if (line->feed1 < moves_common_def.feed_base)
        line->feed1 = moves_common_def.feed_base;
    else if (line->feed1 > line->feed)
        line->feed1 = line->feed;

    if (line->feed0 < moves_common_def.feed_base)
        line->feed0 = moves_common_def.feed_base;
    else if (line->feed0 > line->feed)
        line->feed0 = line->feed;

    bresenham_plan(line);
    line->acc_steps = acceleration(line->feed0, line->feed, line->acceleration, line->len, 0, line->steps);
    line->dec_steps = acceleration(line->feed1, line->feed, line->acceleration, line->len, line->steps, 0);

    if (line->acc_steps + line->dec_steps > line->steps)
    {
        int32_t d = (line->acc_steps + line->dec_steps - line->steps) / 2;
        line->acc_steps -= d;
        line->dec_steps -= d;
        if (line->acc_steps + line->dec_steps < line->steps)
            line->acc_steps += (line->steps - line->acc_steps - line->dec_steps);
    }
    if (line->acc_steps < 0 || line->dec_steps < 0)
    {
        // We can not perform such moving!
        if (line->acc_steps < 0)
            line->acc_steps = 0;
        if (line->dec_steps < 0)
            line->dec_steps = 0;           
    }
}

