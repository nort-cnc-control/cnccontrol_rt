#include "line.h"
#include "moves.h"

#define LINE 0

static int type;

cnc_endstops endstops;
cnc_position position;

steppers_definition def;

void moves_init(steppers_definition definition)
{
	def = definition;
}

int moves_line_to(line_plan *plan)
{
    type = LINE;
    line_move_to(plan);
}

int moves_step_tick(void)
{
	if (type == LINE)
		return line_step_tick();
	return -1;
}

cnc_endstops moves_get_endstops(void)
{
	return def.get_endstops();
}
