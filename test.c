#include <stdio.h>
#include <arc.h>
#include <planner.h>
#include <control.h>
#include <shell.h>
#include "config.h"

static void set_dir(int coord, int dir)
{}

static void make_step(int coord)
{}

static cnc_endstops get_stops(void)
{
    cnc_endstops stops = {
        .stop_x  = 0,
        .stop_y  = 0,
        .stop_z  = 0,
        .probe_z = 0,
    };

    return stops;
}

static void line_started(void)
{}

static void line_finished(void)
{}

static void line_error(void)
{}

void config_steppers(steppers_definition *sd)
{
    sd->set_dir        = set_dir;
    sd->make_step      = make_step;
    sd->get_endstops   = get_stops;
    sd->line_started   = line_started;
    sd->line_finished  = line_finished;
    sd->line_error     = line_error;
}

static void init_steppers(void)
{
    steppers_definition sd = {
        .steps_per_unit = {
            STEPS_PER_MM,
            STEPS_PER_MM,
            STEPS_PER_MM
        },
        .feed_base = FEED_BASE,
        .feed_max = FEED_MAX,
        .es_travel = FEED_ES_TRAVEL,
        .probe_travel = FEED_PROBE_TRAVEL,
        .es_precise = FEED_ES_PRECISE,
        .probe_precise = FEED_PROBE_PRECISE,
        .size = {
            SIZE_X,
            SIZE_Y,
            SIZE_Z,
        },
        .acc_default = ACC,
        .xy_right = XY_RIGHT,
        .yz_right = YZ_RIGHT,
        .zx_right = ZX_RIGHT,
    };
    config_steppers(&sd);
    init_control(sd);
}

void test_arc(void)
{
	fixed x[3] = {FIXED_ENCODE(20), 0, 0};
	planner_arc_to(x, 0, XY, 0, 100, 100, 100, 1, 0);
	int l;
	do {
		l = moves_step_tick();
	} while (l > 0);
}

void send_char(char c)
{
	putchar(c);
}

int main(void)
{
	shell_cbs cbs = {
		.send_char = send_char,
	};
	shell_init(cbs);
 	init_steppers();
	test_arc();	
	return 0;
}

