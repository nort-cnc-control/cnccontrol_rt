#include <math.h>

#include <print.h>
#include <shell.h>

#include "common.h"
#include "line.h"
#include "moves.h"
#include <err.h>

#include <acceleration.h>

#define SQR(x) ((x) * (x))

static line_plan *current_plan;

static struct
{
    int32_t err[3];
    int is_moving;

    acceleration_state acc;

    double start_pos[3];
} current_state;

static steppers_definition def;
void line_init(steppers_definition definition)
{
    def = definition;
}

int line_move_to(line_plan *plan)
{
    int i;

    current_plan = plan;

    if (current_plan->len < 0)
    {
        line_pre_calculate(current_plan);
    }

    for (i = 0; i < 3; i++)
        def.set_dir(i, current_plan->s[i] >= 0);

    if (current_plan->check_break && current_plan->check_break(current_plan->s, current_plan->check_break_data))
        return -E_NEXT;

    if (current_plan->steps == 0)
        return -E_NEXT;

    current_state.acc.acceleration = current_plan->acceleration;
    current_state.acc.feed = current_plan->feed0;
    current_state.acc.target_feed = current_plan->feed;
    current_state.acc.end_feed = current_plan->feed1;
    current_state.acc.type = STATE_ACC;
    current_state.acc.step = 0;
    current_state.acc.total_steps = current_plan->steps;
    current_state.acc.acc_steps = current_plan->acc_steps;
    current_state.acc.dec_steps = current_plan->dec_steps;
    current_state.is_moving = 1;
    for (i = 0; i < 3; i++)
    {
        current_state.err[i] = 0;
        current_state.start_pos[i] = position.pos[i];
    }

    def.line_started();
    return -E_OK;
}

static double make_step(void)
{
    int i;
    if (current_state.acc.step >= current_state.acc.total_steps)
    {
        return -1;
    }
    /* Bresenham */
    def.make_step(current_plan->maxi);
    for (i = 0; i < 3; i++)
    {
        if (i == current_plan->maxi)
            continue;
        current_state.err[i] += abs(current_plan->s[i]);
        if (current_state.err[i] * 2 >= (int32_t)current_plan->steps)
        {
            current_state.err[i] -= (int32_t)current_plan->steps;
            def.make_step(i);
        }
    }

    current_state.acc.step++;
    double len = current_plan->len / current_state.acc.total_steps;

    // Set current position
    double cx[3];
    for (i = 0; i < 3; i++)
    {
        cx[i] = current_state.start_pos[i] +
                current_plan->x[i] * current_state.acc.step / current_state.acc.total_steps;
    }
    moves_set_position(cx);

    return len;
}

int line_step_tick(void)
{
    // Make step
    double len = make_step();

    // Check if we have reached the end
    if (len <= 0)
    {
        current_state.is_moving = 0;
        def.line_finished();
        return -1;
    }

    // Check for endstops
    if (current_plan->check_break && current_plan->check_break(current_plan->s, current_plan->check_break_data))
    {
        current_state.is_moving = 0;
        def.line_finished();
        return -1;
    }

    /* Calculating delay */
    int step_delay = feed2delay(current_state.acc.feed, current_plan->len / current_plan->steps);
    acceleration_process(&current_state.acc, step_delay);
    return step_delay;
}

static void bresenham_plan(line_plan *plan)
{
    int i;
    plan->steps = abs(plan->s[0]);
    plan->maxi = 0;
    if (abs(plan->s[1]) > plan->steps)
    {
        plan->maxi = 1;
        plan->steps = abs(plan->s[1]);
    }

    if (abs(plan->s[2]) > plan->steps)
    {
        plan->maxi = 2;
        plan->steps = abs(plan->s[2]);
    }
}

void line_pre_calculate(line_plan *line)
{
    int j;
    int64_t l = 0;
    for (j = 0; j < 3; j++)
    {
        l += SQR(line->x[j]);
        line->s[j] = line->x[j] * def.steps_per_unit[j];
    }
    line->len = sqrt(l);

    if (line->len == 0)
        return;

    if (line->feed < def.feed_base)
        line->feed = def.feed_base;
    else if (line->feed > def.feed_max)
        line->feed = def.feed_max;

    if (line->feed1 < def.feed_base)
        line->feed1 = def.feed_base;
    else if (line->feed1 > line->feed)
        line->feed1 = line->feed;

    if (line->feed0 < def.feed_base)
        line->feed0 = def.feed_base;
    else if (line->feed0 > line->feed)
        line->feed0 = line->feed;

    bresenham_plan(line);
    line->acc_steps = acceleration_steps(line->feed0, line->feed, line->acceleration, line->len, line->steps);
    line->dec_steps = acceleration_steps(line->feed1, line->feed, line->acceleration, line->len, line->steps);

    if (line->acc_steps + line->dec_steps > line->steps)
    {
        int32_t d = (line->acc_steps + line->dec_steps - line->steps) / 2;
        line->acc_steps -= d;
        line->dec_steps -= d;
        if (line->acc_steps + line->dec_steps < line->steps)
            line->acc_steps += (line->steps - line->acc_steps - line->dec_steps);
    }
}
