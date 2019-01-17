#include <acceleration.h>
#include <common.h>
#include <print.h>
#include <shell.h>

void acceleration_process(acceleration_state *state, int32_t step_delay)
{
    if (state->step >= state->total_steps)
    {
        shell_send_string("debug: finished\n\r");
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
            state->type = STATE_STOP;
        }
    }
    break;
    case STATE_STOP:
        break;
    }
}
