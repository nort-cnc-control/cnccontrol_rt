#include <stdint.h>
#include <math.h>
#include <common.h>

#define SQR(a) ((a) * (a))

cnc_position position;
steppers_definition *moves_common_def;

void moves_common_init(steppers_definition *definition)
{
    moves_common_def = definition;
}

void moves_common_reset(void)
{
    int i;
    for (i = 0; i < 3; i++)
        position.pos[i] = 0;
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

// Find new feed when acceleration
//
// feed.  mm / sec
// acc.   mm / sec^2
// delay. sec
//
// Return: new feed in mm / min
double accelerate(double feed, double acc, double delay)
{
    double df = acc * delay;
    return feed + df;
}

// Find amount of acceleration steps from feed0 to feed1
//
// feed0. mm / sec
// feed1. mm / sec
// acc.   mm / sec^2
// len.   mm
uint32_t acceleration_steps(double feed0,
                            double feed1,
                            double acc,
                            double step_len)
{
    double slen = (SQR(feed1) - SQR(feed0)) / (2*acc);
    return slen / step_len;
}

// Movement functions
void moves_common_set_dir(int i, bool dir)
{
    if (moves_common_def->set_dir)
        moves_common_def->set_dir(i, dir);
}

void moves_common_make_step(int i)
{
    if (moves_common_def->make_step)
        moves_common_def->make_step(i);
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
void moves_common_set_position(int32_t x[3])
{
    int i;
    for (i = 0; i < 3; i++)
    {
        position.pos[i] = x[i];
    }
}


