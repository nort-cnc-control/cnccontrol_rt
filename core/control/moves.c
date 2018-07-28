#include "moves.h"

#define abs(a) ((a) > 0 ? (a) : (-(a))) 

cnc_position position;
int busy;

static int32_t dc[3];
static int32_t err[3];
static int maxi;
static int steps;
static int step;
static int is_moving;

static steppers_definition def;

void init_moves(steppers_definition definition)
{
	def = definition;
}

static void bresenham_plan(void)
{
    int i;
    steps = abs(dc[0]);
    maxi = 0;
    if (abs(dc[1]) > steps)
    {
        maxi = 1;
        steps = abs(dc[1]);
    }

    if (abs(dc[2]) > steps)
    {
        maxi = 2;
        steps = abs(dc[2]);
    }

    for (i = 0; i < 3; i++)
    {
        def.set_dir(i, dc[i] >= 0);
    }
    step = 0;
}

void move_line_to(int32_t x[3])
{
    int i;
    for (i = 0; i < 3; i++)
        dc[i] = x[i] * def.steps_per_unit[i] / 100;
    
    bresenham_plan();
    is_moving = 1;
    def.line_started();
}

int step_tick(void)
{
	int i;
	if (step >= steps)
		return -1;

	def.make_step(maxi);
	for (i = 0; i < 3; i++) {
		if (i == maxi)
			continue;
		err[i] += abs(dc[i]);
		if (err[i] * 2 >= steps) {
			err[i] -= steps;
			def.make_step(i);
		}
	}

	step++;
	if (step == steps) {
        	def.line_finished();
        	return -1;
    	}
    	return 10;
}

void set_speed(int32_t speed)
{

}

void find_begin(int rx, int ry, int rz)
{

}

