#include <control/moves/moves_common/acceleration.h>
#include <control/moves/moves_common/common.h>

void acceleration_process(acceleration_state *state, double step_delay, double t)
{
    state->current_t = t;
    if (state->current_t >= state->end_t)
    {
        state->type = STATE_STOP;
        return;
    }

    switch (state->type)
    {
    case STATE_ACC:
    {
        if (state->current_t >= state->acc_t)
        {
            if (state->current_t < state->dec_t)
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
        if (state->current_t >= state->dec_t)
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

