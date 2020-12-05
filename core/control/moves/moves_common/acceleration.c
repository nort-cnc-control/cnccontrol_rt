#include <control/moves/moves_common/acceleration.h>
#include <control/moves/moves_common/common.h>
#include <math.h>

void acceleration_process(acceleration_state *state, double step_delay, double t)
{
    double cur_dt = fabs(state->current_t - state->start_t);
    double acc_dt = fabs(state->acc_t - state->start_t);
    double dec_dt = fabs(state->dec_t - state->start_t);
    double total_dt = fabs(state->end_t - state->start_t);

    state->current_t = t;
    if (cur_dt >= total_dt)
    {
        state->type = STATE_STOP;
        return;
    }


    switch (state->type)
    {
    case STATE_ACC:
    {
        if (cur_dt >= acc_dt)
        {
            if (cur_dt < dec_dt)
            {
                state->type = STATE_GO;
                state->feed = state->target_feed;
            }
            else
            {
                state->type = STATE_DEC;
            }
        }
        else
        {
            state->feed = accelerate(state->feed, state->acceleration, step_delay);
        }
        break;
    }
    case STATE_GO:
        if (cur_dt >= dec_dt)
        {
            state->type = STATE_DEC;
        }
        break;
    case STATE_DEC:
    {
        state->feed = accelerate(state->feed, -state->acceleration, step_delay);
        if (state->feed < state->end_feed)
        {
            state->feed = state->end_feed;
            state->type = STATE_STOP_COMPLETION;
        }
        break;
    }
    case STATE_STOP:
    case STATE_STOP_COMPLETION:
        break;
    }
}

// Find new feed when acceleration
//
// feed.  mm / sec
// acc.   mm / sec^2
// delay. sec
//
// Return: new feed in mm / min
double accelerate(double feed, double acc, double delay)
{
    double df = acc * delay;
    return feed + df;
}

// Find amount of acceleration steps from feed0 to feed1
//
// feed0. mm / sec
// feed1. mm / sec
// acc.   mm / sec^2
double acceleration(double feed0,
                    double feed1,
                    double acc,
                    double len,
                    double t_start,
                    double t_end)
{
    double mlen = (feed1*feed1 - feed0*feed0) / (2*acc);
    return t_start + (t_end - t_start) * mlen / len;
}

