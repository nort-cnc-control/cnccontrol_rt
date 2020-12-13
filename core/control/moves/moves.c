#include <control/moves/moves.h>
#include <control/moves/moves_line/line.h>
#include <control/moves/moves_arc/arc.h>

static steppers_definition *def;

static enum {
    MOVE_NONE = 0,
    MOVE_LINE,
    MOVE_ARC,
} current_move_type;

static bool ready = true;

void moves_break(void)
{
    current_move_type = MOVE_NONE;
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
    ready = true;
    current_move_type = MOVE_LINE;
    return line_move_to(plan);
}

int moves_arc_to(arc_plan *plan)
{
    ready = true;
    current_move_type = MOVE_ARC;
    return arc_move_to(plan);
}

int32_t moves_step_tick(void)
{
    /* Check endstops */
    bool es;
    if (current_move_type == MOVE_LINE)
    {
        es = line_check_endstops();
    }
    else if (current_move_type == MOVE_ARC)
    {
        es = arc_check_endstops();
    }
    if (es)
    {
        moves_common_endstops_touched();
        return -1;
    }
   
    if (ready)
    {
        /* Normal movement */
        int res;
        if (current_move_type == MOVE_LINE)
        {
            res = line_step_tick();
        }
        else if (current_move_type == MOVE_ARC)
        {
            res = arc_step_tick();
        }

        if (res == -E_OK)
        {
            double len;
            double dt;
            ready = moves_common_make_steps(&len);
            if (current_move_type == MOVE_LINE)
            {
                dt = line_acceleration_process(len);
            }
            else if (current_move_type == MOVE_ARC)
            {
                dt = arc_acceleration_process(len);
            }
            return dt * 1000000UL;
        }
        else if (res == -E_NEXT)
        {
            moves_common_line_finished();
            return -1;
        }
    }
    else
    {
        /* Move to target position */
        double len, dt;
        ready = moves_common_make_steps(&len);
        if (current_move_type == MOVE_LINE)
        {
            dt = len / line_movement_feed();
        }
        else if (current_move_type == MOVE_ARC)
        {
            dt = len / arc_movement_feed();
        }
        return dt * 1000000UL;
    }
    return -1;
}

cnc_endstops moves_get_endstops(void)
{
    return def->get_endstops();
}

