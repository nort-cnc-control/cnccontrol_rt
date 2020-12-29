#include "config.h"

#ifdef CONFIG_LIBCORE
#include <control/moves/moves.h>
#include <control/planner/planner.h>
#include <control/control.h>
#include <output/output.h>
#include "steppers.h"
#endif

#include <shell.h>

#include "platform.h"
#include "net.h"


#ifdef CONFIG_LIBCORE
static void init_steppers(void)
{
    gpio_definition gd;

    steppers_definition sd = {};
    steppers_config(&sd, &gd);
    init_control(&sd, &gd);
}
#endif

/* main */


int main(void)
{
    hardware_setup();

#ifdef CONFIG_LIBCORE
    init_steppers();
    planner_lock();
    moves_reset();
#endif

    while (!net_ready())
    {
        net_receive();
    }

    shell_add_message("Hello", -1);

    while (true)
    {
#ifdef CONFIG_LIBCORE
        planner_pre_calculate();
#endif
	hardware_loop();
    }

    return 0;
}

