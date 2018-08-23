#include "config.h"

#include <shell/shell.h>
#include <shell/print.h>
#include <control/control.h>
#include <control/moves.h>
#include <control/planner.h>

void hardware_setup(void);

void config_steppers(steppers_definition *sd);

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
			SIZE_X * 100,
			SIZE_Y * 100,
			SIZE_Z * 100,
		},
		.acc_default = ACC,
	};
	config_steppers(&sd);
	init_planner(sd);
}

int main(void)
{
	hardware_setup();
	init_steppers();
	shell_send_string("Hello\r\n");
        while (1) {
		planner_pre_calculate();
	}

        return 0;
}

