#include <acceleration.h>
#include <common.h>

void acceleration_process(acceleration_state *state, double step_delay)
{
    if (state->step >= state->total_steps)
    {
        state->type = STATE_STOP;
        return;
    }
    switch (state->type)
    {
    case STATE_ACC:
    {
        if (state->step >= state->acc_steps)
        {
            if (state->step < state->total_steps - state->dec_steps)
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
        if (state->step >= state->total_steps - state->dec_steps)
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
    state->step++;
}

