#include "config.h"

#include <shell/shell.h>
#include <shell/print.h>
#include <control/control.h>
#include <control/moves.h>
#include <control/planner.h>

void hardware_setup(void);

int main(void)
{
	hardware_setup();
	shell_send_string("Hello\r\n");
        while (1) {
		planner_pre_calculate();
	}

        return 0;
}

