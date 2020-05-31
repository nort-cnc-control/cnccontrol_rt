#include "line.h"
#include "arc.h"
#include "moves.h"

static steppers_definition *def;

static enum {
    MOVE_NONE = 0,
    MOVE_LINE,
    MOVE_ARC,
} current_move_type;

void moves_break(void)
{
    current_move_type = MOVE_NONE;
    moves_common_reset();
}

void moves_init(steppers_definition *definition)
{
    def = definition;
    current_move_type = MOVE_NONE;
    moves_common_init(definition);
    moves_common_reset();
}

void moves_reset(void)
{
    current_move_type = MOVE_NONE;
    moves_common_reset();
}

int moves_line_to(line_plan *plan)
{
    int res = line_move_to(plan);
    if (res == -E_OK)
        current_move_type = MOVE_LINE;
    return res;
}

int moves_arc_to(arc_plan *plan)
{
    int res = arc_move_to(plan);
    if (res == -E_OK)
        current_move_type = MOVE_ARC;
    return res;
}

int moves_step_tick(void)
{
    if (current_move_type == MOVE_LINE)
        return line_step_tick();
    else if (current_move_type == MOVE_ARC)
        return arc_step_tick();
    return -10;
}

cnc_endstops moves_get_endstops(void)
{
    return def->get_endstops();
}

