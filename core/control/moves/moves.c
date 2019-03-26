#include "line.h"
#include "arc.h"
#include "moves.h"

static enum {
    MOVE_NONE = 0,
    MOVE_LINE,
    MOVE_ARC,
} current_move_type;

cnc_position position;

static struct
{
    double delta[3];
    double target[3];
} state;

static steppers_definition def;

static void add_move(double x[3])
{
    int i;
    for (i = 0; i < 3; i++)
    {
        state.target[i] += x[i];
        state.delta[i] = state.target[i] - position.pos[i];
    }
}

void moves_reset(void)
{
    int i;
    for (i = 0; i < 3; i++)
    {
        state.target[i] = position.pos[i];
        state.delta[i] = 0;
    }
}

void moves_set_position(double x[3])
{
    int i;
    for (i = 0; i < 3; i++)
    {
        position.pos[i] = x[i];
        state.delta[i] = state.target[i] - position.pos[i];
    }
}

void moves_init(steppers_definition definition)
{
    def = definition;
    line_init(def);
    arc_init(def);
    moves_reset();
}

int moves_line_to(line_plan *plan)
{
    int i;
    for (i = 0; i < 3; i++)
        plan->x[i] += state.delta[i];
    add_move(plan->x);
    current_move_type = MOVE_LINE;
    return line_move_to(plan);
}

int moves_arc_to(arc_plan *plan)
{
    int i;
    //for (i = 0; i < 3; i++)
    //    plan->x[i] += state.delta[i];
    add_move(plan->x);
    current_move_type = MOVE_ARC;
    return arc_move_to(plan);
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
    return def.get_endstops();
}
