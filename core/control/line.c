#include <math/math.h>

#include <shell/print.h>
#include <shell/shell.h>
#include "line.h"
#include "moves.h"

#define abs(a) ((a) > 0 ? (a) : (-(a))) 

enum {
	STATE_STOP = 0,
	STATE_ACC,
	STATE_GO,
	STATE_DEC,
} state;

static const int64_t feed_k = 1000;
typedef int64_t fixed;
#define FIXED_ENCODE(x) ((x) * feed_k)
#define FIXED_DECODE(x) ((x) / feed_k)

static int32_t acceleration;

static uint64_t len;
static int32_t dc[3], dx[3];
static int32_t start_position[3];
static int32_t err[3];
static int maxi;
static int32_t steps;
static int32_t steps_acc;
static int32_t steps_dec;
static int32_t step;

static fixed feed_next;
static fixed feed_end;
static volatile int is_moving;

static int32_t feed;

static steppers_definition def;

void line_init(steppers_definition definition)
{
	def = definition;
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
	// feed - (mm * feed_k) / min
	// acc - mm / s^2
	// delay = us
	return feed + acc * delay / (1000000 / feed_k / 60);
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

int line_move_to(line_plan *plan)
{
	int i;

	if (plan->len < 0)
	{
		line_pre_calculate(plan);
	}

	for (i = 0; i < 3; i++)
	{
		dx[i] = plan->x[i];
		dc[i] = plan->s[i];
		def.set_dir(i, dc[i] >= 0);
	}

	endstops = def.get_endstops();
	if (endstops.stop_x && dx[0] < 0)
	{
		def.line_started();
		def.line_error();
		return -2;
	}
	if (endstops.stop_y && dx[1] < 0)
	{
		def.line_started();
		def.line_error();
		return -2;
	}
	if (endstops.stop_z && dx[2] < 0)
	{
		def.line_started();
		def.line_error();
		return -2;
	}

	len = plan->len;
	maxi = plan->maxi;
	steps = plan->steps;
	feed = plan->feed;
	acceleration = plan->acceleration;
	feed_next = FIXED_ENCODE(plan->feed0);
	feed_end  = FIXED_ENCODE(plan->feed1);

	steps_acc = plan->acc_steps;
	steps_dec = plan->dec_steps;

	if (steps == 0)
	{
		def.line_started();
		def.line_finished();
		return 0;
	}

	state = STATE_ACC;
	is_moving = 1;
	step = 0;
	for (i = 0; i < 3; i++)
		start_position[i] = position.pos[i];
	def.line_started();
	return 0;
}

int line_step_tick(void)
{
	int ex;
	int i;
	int32_t step_delay;
	if (step >= steps)
	{
		return -1;
	}
	cnc_endstops nes = def.get_endstops();

	ex = 0;
	/* check endstops */
	if (dx[0] < 0 && nes.stop_x && !endstops.stop_x) {
		ex = 1;
	}
	if (dx[1] < 0 && nes.stop_y && !endstops.stop_y) {
		ex = 1;
	}
	if (dx[2] < 0 && nes.stop_z && !endstops.stop_z) {
		ex = 1;
	}

	if (ex) {
		is_moving = 0;
		def.line_finished();
		return -1;
	}

	endstops = nes;

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
	int32_t cx[3];
	for (i = 0; i < 3; i++)
	{
		cx[i] = start_position[i] + dx[i] * step / steps;
	}
	moves_set_position(cx);

	if (step >= steps) {
		is_moving = 0;
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

static void bresenham_plan(line_plan *plan)
{
	int i;
	plan->steps = abs(plan->s[0]);
	plan->maxi = 0;
	if (abs(plan->s[1]) > plan->steps)
	{
		plan->maxi = 1;
		plan->steps = abs(plan->s[1]);
	}

	if (abs(plan->s[2]) > plan->steps)
	{
		plan->maxi = 2;
		plan->steps = abs(plan->s[2]);
	}
}

void line_pre_calculate(line_plan *line)
{
	int j;
	int64_t l = 0;
	for (j = 0; j < 3; j++)
	{
		l += ((int64_t)line->x[j]) * line->x[j];
		line->s[j] = line->x[j] * def.steps_per_unit[j] / 100;
	}
	line->len = isqrt(l);
	
	if (line->feed < def.feed_base)
		line->feed = def.feed_base;
	else if (line->feed > def.feed_max)
		line->feed = def.feed_max;

	if (line->feed1 < def.feed_base)
		line->feed1 = def.feed_base;
	else if (line->feed1 > line->feed)
		line->feed1 = line->feed;

	if (line->feed0 < def.feed_base)
		line->feed0 = def.feed_base;
	else if (line->feed0 > line->feed)
		line->feed0 = line->feed;

	bresenham_plan(line);


	line->acc_steps = acc_steps(line->feed0, line->feed, line->acceleration, line->len, line->steps);
	line->dec_steps = acc_steps(line->feed1, line->feed, line->acceleration, line->len, line->steps);

	if (line->acc_steps + line->dec_steps > line->steps)
	{
		int32_t d = (line->acc_steps + line->dec_steps - line->steps)/2;
		line->acc_steps -= d;
		line->dec_steps -= d;
		if (line->acc_steps + line->dec_steps < line->steps)
			line->acc_steps += (line->steps - line->acc_steps - line->dec_steps);
	}
}

