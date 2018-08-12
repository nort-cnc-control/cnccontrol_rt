#include "line.h"
#include "moves.h"

#define LINE 0
#define CIRCLE_CW 1
#define CIRCLE_CCW 2

static int type;

cnc_endstops endstops;
cnc_position position;

steppers_definition def;

void moves_init(steppers_definition definition)
{
	def = definition;
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

