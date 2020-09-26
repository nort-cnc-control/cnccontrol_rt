#include "config.h"

#include <control/moves/moves.h>
#include <control/planner/planner.h>
#include <control/control.h>
#include <output/output.h>

#include "platform.h"
#include "steppers.h"
#include "net.h"

static void init_steppers(void)
{
    gpio_definition gd;

    steppers_definition sd = {
        .steps_per_unit = {
            STEPS_PER_MM,
            STEPS_PER_MM,
            STEPS_PER_MM
        },
        .feed_base = FEED_BASE/60.0,
        .feed_max = FEED_MAX/60.0,
        .es_travel = FEED_ES_TRAVEL/60.0,
        .probe_travel = FEED_PROBE_TRAVEL/60.0,
        .es_precise = FEED_ES_PRECISE/60.0,
        .probe_precise = FEED_PROBE_PRECISE/60.0,
        .size = {
            SIZE_X,
            SIZE_Y,
            SIZE_Z,
        },
        .acc_default = ACC,
    };
    steppers_config(&sd, &gd);
    init_control(&sd, &gd);
}

/* main */


int main(void)
{
    hardware_setup();
    init_steppers();

    planner_lock();
    moves_reset();

    while (!net_ready())
    {
        net_receive();
    }

    output_control_write("Hello", -1);

    while (true)
    {
        planner_pre_calculate();
	hardware_loop();
    }

    return 0;
}

