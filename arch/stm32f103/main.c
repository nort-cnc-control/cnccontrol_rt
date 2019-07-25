#include "config.h"

#include <moves.h>
#include <planner.h>
#include <control.h>
#include <output.h>

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
            SIZE_X,
            SIZE_Y,
            SIZE_Z,
        },
        .acc_default = ACC,
    };
    config_steppers(&sd);
    init_control(sd);
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
        planner_pre_calculate();
    }

    return 0;
}
