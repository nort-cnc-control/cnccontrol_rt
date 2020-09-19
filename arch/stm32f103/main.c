#include "config.h"

#include <control/moves/moves.h>
#include <control/planner/planner.h>
#include <control/control.h>
#include <output/output.h>

void hardware_setup(void);
void poll_net(void);
void hardware_loop(void);
void config_steppers(steppers_definition *sd, gpio_definition *gd);

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
    config_steppers(&sd, &gd);
    init_control(&sd, &gd);
}

int main(void)
{
    hardware_setup();
    init_steppers();
    
    planner_lock();
    moves_reset();

    output_control_write("Hello", -1);

    while (true)
    {
//        poll_net();
        planner_pre_calculate();
	hardware_loop();
    }

    return 0;
}
