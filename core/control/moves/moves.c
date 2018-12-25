#include "line.h"
#include "moves.h"

static enum {
    MOVE_NONE = 0,
    MOVE_LINE,
} current_move_type;

cnc_position position;

static steppers_definition def;



void moves_set_position(int32_t x[3])
{
    int i;
    for (i = 0; i < 3; i++)
        position.pos[i] = x[i];
}

void moves_init(steppers_definition definition)
{
    def = definition;
    line_init(def);
}

int moves_line_to(line_plan *plan)
{
    current_move_type = MOVE_LINE;
    return line_move_to(plan);
}

int moves_step_tick(void)
{
    if (current_move_type == MOVE_LINE)
        return line_step_tick();
    return -10;
}

cnc_endstops moves_get_endstops(void)
{
    return def.get_endstops();
}

