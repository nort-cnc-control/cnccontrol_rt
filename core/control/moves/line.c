#include <math.h>
#include <fixed.h>

#include <print.h>
#include <shell.h>

#include "common.h"
#include "line.h"
#include "moves.h"
#include <err.h>

static line_plan *current_plan;

static struct {
    int32_t err[3];
    uint32_t step;

    fixed feed;
    fixed feed_end;

    int is_moving;

    enum {
        STATE_STOP = 0,
        STATE_ACC,
        STATE_GO,
        STATE_DEC,
    } state;

    int32_t start_pos[3];
} current_state;

static steppers_definition def;
void line_init ( steppers_definition definition )
{
    def = definition;
}

int line_move_to ( line_plan *plan )
{
    int i;

    current_plan = plan;

    if ( current_plan->len < 0 ) {
        line_pre_calculate ( current_plan );
    }

    for ( i = 0; i < 3; i++ )
        def.set_dir ( i, current_plan->s[i] >= 0 );

    if ( current_plan->check_break && current_plan->check_break (current_plan->s, current_plan->check_break_data) )
        return -E_NEXT;

    if ( current_plan->steps == 0 )
        return -E_NEXT;

	current_state.feed = current_plan->feed0;
    current_state.state = STATE_ACC;
    current_state.is_moving = 1;
    current_state.step = 0;
    for ( i = 0; i < 3; i++ )
        current_state.start_pos[i] = position.pos[i];
    def.line_started();
    return -E_OK;
}

int line_step_tick ( void )
{
    int ex;
    int i;
    int32_t step_delay;
    if ( current_state.step >= current_plan->steps ) {
        current_state.is_moving = 0;
        def.line_finished();
        return -1;
    }

    if ( current_plan->check_break && current_plan->check_break (current_plan->s, current_plan->check_break_data ) ) {
        current_state.is_moving = 0;
        def.line_finished();
        return -1;
    }

    /* Bresenham */
    def.make_step ( current_plan->maxi );
    for ( i = 0; i < 3; i++ ) {
        if ( i == current_plan->maxi )
            continue;
        current_state.err[i] += abs ( current_plan->s[i] );
        if ( current_state.err[i] * 2 >= current_plan->steps ) {
            current_state.err[i] -= current_plan->steps;
            def.make_step ( i );
        }
    }

    current_state.step++;
    int32_t cx[3];
    for ( i = 0; i < 3; i++ ) {
        cx[i] = current_state.start_pos[i] + current_plan->x[i] * current_state.step / current_plan->steps;
    }
    moves_set_position ( cx );

    /* Calculating delay */
    step_delay = feed2delay ( current_state.feed, current_plan->len, current_plan->steps );
    if ( current_state.step == current_plan->steps )
        current_state.state = STATE_STOP;
    switch ( current_state.state ) {
    case STATE_ACC:
        if ( current_state.step >= current_plan->acc_steps ) {
            if ( current_state.step < current_plan->steps - current_plan->dec_steps ) {
                current_state.state = STATE_GO;
                current_state.feed = current_plan->feed;
            } else {
                current_state.state = STATE_DEC;
	    }
        } else {
            current_state.feed = accelerate ( current_state.feed, current_plan->acceleration, step_delay );
        }
        break;
    case STATE_GO:
        if ( current_state.step >= current_plan->steps - current_plan->dec_steps ) {
            current_state.state = STATE_DEC;
        }
        break;
    case STATE_DEC:
	{
	int acc = current_plan->acceleration;
        current_state.feed = accelerate ( current_state.feed, -acc, step_delay );
	if ( current_state.feed < current_plan->feed1 ) {
            current_state.feed = current_plan->feed1;
            current_state.state = STATE_STOP;
        }
	}
        break;
    case STATE_STOP:
        break;
    }

    return step_delay;
}

static void bresenham_plan ( line_plan *plan )
{
    int i;
    plan->steps = abs ( plan->s[0] );
    plan->maxi = 0;
    if ( abs ( plan->s[1] ) > plan->steps ) {
        plan->maxi = 1;
        plan->steps = abs ( plan->s[1] );
    }

    if ( abs ( plan->s[2] ) > plan->steps ) {
        plan->maxi = 2;
        plan->steps = abs ( plan->s[2] );
    }
}

void line_pre_calculate ( line_plan *line )
{
    int j;
    int64_t l = 0;
    for ( j = 0; j < 3; j++ ) {
        l += ( ( int64_t ) line->x[j] ) * line->x[j];
        line->s[j] = FIXED_DECODE(line->x[j] * def.steps_per_unit[j]);
    }
    line->len = isqrt ( l );

    if ( line->len == 0 )
        return;

    if ( line->feed < def.feed_base )
        line->feed = def.feed_base;
    else if ( line->feed > def.feed_max )
        line->feed = def.feed_max;

    if ( line->feed1 < def.feed_base )
        line->feed1 = def.feed_base;
    else if ( line->feed1 > line->feed )
        line->feed1 = line->feed;

    if ( line->feed0 < def.feed_base )
        line->feed0 = def.feed_base;
    else if ( line->feed0 > line->feed )
        line->feed0 = line->feed;

    bresenham_plan ( line );
/*
    shell_print_dec(line->feed0);
    shell_send_char(',');
    shell_print_dec(line->feed);
    shell_send_char(',');
    shell_print_dec(line->feed1);
    shell_send_char('\n');
  */  
//    line->acc_steps = acceleration_steps ( line->feed0, line->feed, line->acceleration, line->len, line->steps );
//    line->dec_steps = acceleration_steps ( line->feed1, line->feed, line->acceleration, line->len, line->steps );

    line->acc_steps = 0;
    line->dec_steps = 0;

    if ( line->acc_steps + line->dec_steps > line->steps ) {
        int32_t d = ( line->acc_steps + line->dec_steps - line->steps ) / 2;
        line->acc_steps -= d;
        line->dec_steps -= d;
        if ( line->acc_steps + line->dec_steps < line->steps )
            line->acc_steps += ( line->steps - line->acc_steps - line->dec_steps );
    }
}

