#include <math/math.h>

#include <shell/print.h>
#include <shell/shell.h>
#include "moves.h"

#define abs(a) ((a) > 0 ? (a) : (-(a))) 

enum {
	STATE_STOP = 0,
	STATE_ACC,
	STATE_GO,
	STATE_DEC,
} state;

cnc_position position;
int busy;

static const int64_t feed_k = 1000;
typedef int64_t fixed;
#define FIXED_ENCODE(x) ((x) * feed_k)
#define FIXED_DECODE(x) ((x) / feed_k)

uint64_t len;
static int32_t dc[3], dx[3];

static int32_t err[3];
static int maxi;
static int32_t steps;
static int32_t steps_acc;
static int32_t steps_dec;
static int32_t step;

static int feed;
static fixed feed_next;
static fixed feed_end;
static int is_moving;

static int32_t acceleration;
static steppers_definition def;


void init_moves(steppers_definition definition, int32_t acc)
{
	acceleration = acc;
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

// len is measured in 0.01 mm
// feed in mm / min
// delay in usec
static uint32_t feed_to_delay(uint32_t feed, uint32_t len, uint32_t steps)
{
	uint32_t k = 100 * len / steps;
	if (feed == 0)
		feed = 1;

	//return 1000000 * len * 60  / (feed * 100 * steps);
	return k * 60 * 100 / feed;
}

static fixed accelerate(fixed feed, int32_t acc, int32_t delay)
{
	return feed + acc * delay / (1000000 / feed_k);
}

static int32_t acc_steps(int32_t f0, int32_t f1, int32_t acc, int32_t len, int32_t steps)
{
	uint32_t s = 0;
	fixed fe  = FIXED_ENCODE(f0);
	fixed f1e = FIXED_ENCODE(f1);
	while (fe < f1e)
	{
		uint32_t delay = feed_to_delay(FIXED_DECODE(fe), len, steps);
		fe = accelerate(fe, acc, delay);
		s++;
	}
	return s;
}

void move_line_to(int32_t x[3], int32_t feed0, int32_t feed1)
{
    	int i;
	len = 0;
    	for (i = 0; i < 3; i++)
    	{
		len += x[i]*x[i];
	    	dx[i] = x[i];
		dc[i] = x[i] * def.steps_per_unit[i] / 100;
    	}
	bresenham_plan();
    	if (steps == 0)
		return;

	len = isqrt(len);
//	len = abs(dx[maxi]);

	if (feed1 < def.feed_base)
		feed1 = def.feed_base;
	if (feed1 > def.feed_max)
		feed1 = def.feed_max;

	if (feed0 < def.feed_base)
		feed0 = def.feed_base;
	if (feed0 > def.feed_max)
		feed0 = def.feed_max;

	steps_acc = acc_steps(feed0, feed, acceleration, len, steps);
	steps_dec = acc_steps(feed1, feed, acceleration, len, steps);

	feed_next = FIXED_ENCODE(feed0);
	feed_end  = FIXED_ENCODE(feed1);
	if (steps_acc + steps_dec >= steps)
	{
		int32_t d = (steps_acc + steps_dec - steps)/2;
		steps_acc -= d;
		steps_dec -= d;
		if (steps_acc + steps_dec < steps) {
			steps_acc += (steps - steps_acc - steps_dec);
		}
	}

	state = STATE_ACC;
	is_moving = 1;
	def.line_started();
}

int step_tick(void)
{
	int i;
	int32_t step_delay;
	if (step >= steps)
		return -1;

	/* Bresenham */
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
		for (i = 0; i < 3; i++)
			position.pos[i] += dx[i];
        	def.line_finished();
        	return -1;
    	}
	
	/* Calculating delay */
	step_delay = feed_to_delay(FIXED_DECODE(feed_next), len, steps);
	switch (state)
	{
	case STATE_ACC:
		if (step >= steps_acc) {
			if (step < steps - steps_dec) {
				state = STATE_GO;
				feed_next = FIXED_ENCODE(feed);
			} else {
				state = STATE_DEC;
			}
		} else {
			feed_next = accelerate(feed_next, acceleration, step_delay);
		}
		break;
	case STATE_GO:
		if (step >= steps - steps_dec) {
			state = STATE_DEC;
		}
		break;
	case STATE_DEC:
		feed_next = accelerate(feed_next, -acceleration, step_delay);
		if (feed_next < feed_end)
		{
			feed_next = feed_end;
			state = STATE_STOP;
		}
		break;
	case STATE_STOP:
		break;
	}
	
    	return step_delay;
}

void set_speed(int32_t speed)
{
	feed = speed;
	if (feed < def.feed_base)
		feed = def.feed_base;
	else if (feed > def.feed_max)
		feed = def.feed_max;
}

void set_acceleration(int32_t acc)
{
	acceleration = acc;
}

void find_begin(int rx, int ry, int rz)
{

}

cnc_endstops moves_get_endstops(void)
{
	return def.get_endstops();
}

