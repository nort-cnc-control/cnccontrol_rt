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

uint32_t len;
static int32_t dc[3], dx[3];

static int32_t err[3];
static int maxi;
static int steps;
static int steps_acc;
static int steps_dec;
static int step;
static int step_delay;

static const int feed_k = 1000;
static int feed;
static int feed_next;
static int feed_end;
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

// len is measured in 0.01 mm
// feed in mm / min
// delay in usec
uint32_t feed_to_delay(uint32_t feed, uint32_t len, uint32_t steps)
{
	uint32_t k = 100 * len / steps;
	if (feed == 0)
		feed = 1;

	//return 1000000 * len * 60  / (feed * 100 * steps);
	return k * 60 * 100 / feed;
}

static int32_t accelerate(int32_t feed, int32_t acc, int32_t delay)
{
	return feed + acc * delay / (1000000 / feed_k);
}

static int32_t acc_steps(int32_t f0, int32_t f1, int32_t acc, int32_t len, int32_t steps)
{
	uint32_t s = 0;
	uint32_t f = f0 * feed_k;
	while (f < f1 * feed_k)
	{
		uint32_t delay = feed_to_delay(f / feed_k, len, steps);
		f = accelerate(f, acc, delay);
		s++;
	}
	return s;
}

void move_line_to(int32_t x[3], int32_t feed0, int32_t feed1)
{
	int32_t t;
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

	
	if (feed1 < def.feed_base)
		feed1 = def.feed_base;
	if (feed0 < def.feed_base)
		feed0 = def.feed_base;
	feed_next = feed0 * feed_k;
	feed_end = feed1 * feed_k;

	steps_acc = acc_steps(feed0, feed, def.acceleration, len, steps);
	steps_dec = acc_steps(feed1, feed, def.acceleration, len, steps);

	if (steps_acc + steps_dec >= steps)
	{
		int32_t d = (steps_acc + steps_dec - steps)/2;
		steps_acc -= d;
		steps_dec -= d;
		if (steps_acc + steps_dec < steps) {
			steps_acc += (steps - steps_acc - steps_dec);
		}
	}
	shell_print_dec(steps_acc);
	shell_send_char(' ');	
	shell_print_dec(steps - steps_acc - steps_dec);
	shell_send_char(' ');
	shell_print_dec(steps_dec);
	shell_send_string("\r\n");

	state = STATE_ACC;
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
		for (i = 0; i < 3; i++)
			position.pos[i] += dx[i];
        	def.line_finished();
        	return -1;
    	}
	
	step_delay = feed_to_delay(feed_next / feed_k, len, steps);
	switch (state)
	{
	case STATE_ACC:
		if (step >= steps_acc) {
			state = STATE_GO;
		} else {
			feed_next = accelerate(feed_next, def.acceleration, step_delay);
		}
		break;
	case STATE_GO:
		if (step >= steps - steps_dec) {
			state = STATE_DEC;
		}
		break;
	case STATE_DEC:
		feed_next = accelerate(feed_next, -def.acceleration, step_delay);
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

void find_begin(int rx, int ry, int rz)
{

}

cnc_endstops moves_get_endstops(void)
{
	return def.get_endstops();
}

