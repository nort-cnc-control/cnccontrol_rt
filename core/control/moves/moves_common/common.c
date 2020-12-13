#include <stdint.h>
#include <math.h>

#include <control/moves/moves_common/common.h>

#define SQR(a) ((a) * (a))

static double moves_len[2][2][2] = {};

cnc_position position;
steppers_definition *moves_common_def;

void moves_common_init(steppers_definition *definition)
{
    moves_common_def = definition;
    int x, y, z;
    for (z = 0; z < 2; z++)
    for (y = 0; y < 2; y++)
    for (x = 0; x < 2; x++)
    {
        double sx = x/moves_common_def->steps_per_unit[0];
        double sy = y/moves_common_def->steps_per_unit[1];
        double sz = z/moves_common_def->steps_per_unit[2];
        moves_len[z][y][x] = sqrt(sx*sx + sy*sy + sz*sz);
    }
}

void moves_common_reset(void)
{
    int i;
    for (i = 0; i < 3; i++)
    {
        position.target_pos[i] = 0;
        position.pos[i] = 0;
    }
}

// Find delay between ticks
//
// feed.  mm / sec
// len.   mm
//
// Return: delay. sec
double feed2delay(double feed, double step_len)
{
    if (feed < 0.001)
        feed = 0.001;
    return step_len / feed;
}


// Movement functions
void moves_common_set_dir(int i, bool dir)
{
    if (dir)
        position.dir[i] = 1;
    else
        position.dir[i] = -1;
    if (moves_common_def->set_dir)
        moves_common_def->set_dir(i, dir);
}

void moves_common_schedule_step(int i, int dir)
{
    position.target_pos[i] += dir;
}

void moves_common_make_step(int i)
{
    if (moves_common_def->make_step)
        moves_common_def->make_step(i);
}

static int clip(int d)
{
    if (d < 0)
        return -1;
    if (d > 0)
        return 1;
    return 0;
}

bool moves_common_make_steps(double *len)
{
    int i;
    bool ready = true;
    int8_t delta[3];
    for (i = 0; i < 3; i++)
    {
        delta[i] = position.target_pos[i] - position.pos[i];
        int d = clip(delta[i]);
        if (d != delta[i])
            ready = false;
        moves_common_set_dir(i, d >= 0);
        if (d != 0)
        {
            moves_common_make_step(i);
            position.pos[i] += d;
        }
    }
    if (len != NULL)
        *len = moves_common_step_len(delta[0], delta[1], delta[2]);
    return ready;
}

void moves_common_line_started(void)
{
    if (moves_common_def->line_started)
        moves_common_def->line_started();
}

void moves_common_endstops_touched(void)
{
    if (moves_common_def->endstops_touched)
        moves_common_def->endstops_touched();
}

void moves_common_line_finished(void)
{
    if (moves_common_def->line_finished)
        moves_common_def->line_finished();
}

// State
void moves_common_set_position(const int32_t *x)
{
    int i;
    for (i = 0; i < 3; i++)
    {
        position.pos[i] = x[i];
        position.target_pos[i] = x[i];
    }
}


double moves_common_step_len(int8_t dx, int8_t dy, int8_t dz)
{
    dx = (dx != 0);
    dy = (dy != 0);
    dz = (dz != 0);
    return moves_len[dz][dy][dx];
}

