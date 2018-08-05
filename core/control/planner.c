#include <string.h>
#include <stddef.h>
#include "moves.h"
#include "planner.h"
#include <shell/shell.h>
#include <shell/print.h>
#include <math/math.h>

#define QUEUE_SIZE 50

steppers_definition def;
extern cnc_position position;

static volatile int search_begin;

static void (*finish_action)(void);

typedef enum {
	ACTION_LINE = 0,
	ACTION_FUNCTION,
} action_type;

typedef struct {
	action_type type;
	line_plan line;
	void (*f)(void);
} action_plan;

static action_plan plan[QUEUE_SIZE];
static int plan_len;

int empty_slots(void)
{
	return QUEUE_SIZE - plan_len;
}

static void line_started(void)
{
}

static void pop_cmd(void)
{
	int i;
	memmove(plan, &plan[1], (--plan_len)*sizeof(plan[0]));
}

static void get_cmd(void)
{
	if (plan_len == 0)
	{
		return;
	}

	switch (plan[0].type) {
	case ACTION_LINE:
		def.line_started();
		move_line_to(&(plan[0].line));
		break;
	case ACTION_FUNCTION:
		plan[0].f();
		pop_cmd();
		get_cmd();
		break;
	}
}

static void line_finished(void)
{
	def.line_finished();
	pop_cmd();
	get_cmd();
}

static void line_error(void)
{
	line_finished();
}

void init_planner(steppers_definition pd)
{
	search_begin = 0;
	finish_action = NULL;
	steppers_definition sd = pd;
	def = pd;
	sd.line_started = line_started;
	sd.line_error = line_error;
	sd.line_finished = line_finished;
	init_moves(sd);
}

static int32_t feed_proj(int32_t px[3], int32_t x[3], int32_t f)
{
	int i;
	int64_t s, pl = 0, l;
	for (i = 0; i < 3; i++)
	{
		pl += ((int64_t)px[i]) * px[i];
		l += ((int64_t)x[i]) * x[i];
	}
	pl = isqrt(pl);
	l = isqrt(l);

	for (i = 0; i < 3; i++)
	{
		s += ((int64_t)px[i]) * x[i] / pl / l;
	}

	return s * f;
}

int planner_line_to(int32_t x[3], int feed, int32_t f0, int32_t f1, int32_t acc)
{
	action_plan *prev, *cur;
	if (plan_len >= QUEUE_SIZE)
		return -1;

	if (x[0] == 0 && x[1] == 0 && x[2] == 0)
		return QUEUE_SIZE - plan_len;

	cur = &plan[plan_len];
	cur->type = ACTION_LINE;
	cur->line.x[0] = x[0];
	cur->line.x[1] = x[1];
	cur->line.x[2] = x[2];
	cur->line.feed = feed;
	cur->line.feed0 = f0;
	cur->line.feed1 = f1;
	cur->line.acceleration = acc;
	cur->line.len = -1;
	cur->line.acc_steps = -1;
	cur->line.dec_steps = -1;
	plan_len++;

	if (plan_len == 1) {
		get_cmd();
	}
	return QUEUE_SIZE - plan_len;
}

int planner_function(void (*f)(void))
{
	if (plan_len >= QUEUE_SIZE)
		return -1;

	plan[plan_len].type = ACTION_FUNCTION;
	plan[plan_len].f = f;
	plan_len++;

	if (plan_len == 1) {
		get_cmd();
	}
	return QUEUE_SIZE - plan_len;
}

static int srx, sry, srz; 

void set_pos_0(void)
{
	if (srx)
		position.pos[0] = 0;
	if (sry)
		position.pos[1] = 0;
	if (srz)
		position.pos[2] = 0;
}

void planner_find_begin(int rx, int ry, int rz)
{
	srx = rx;
	sry = ry;
	srz = rz;
	if (rx) {
		int32_t x[3] = {-def.size[0], 0, 0};
		planner_line_to(x, 600, 0, 0, def.acc_default);
		x[0] = 2*100;
		planner_line_to(x, 100, 0, 0, def.acc_default);
		x[0] = -10*100;
		planner_line_to(x, 30, 0, 0, def.acc_default);
	}
	if (ry) {
		int32_t x[3] = {0, -def.size[1], 0};
		planner_line_to(x, 600, 0, 0, def.acc_default);
		x[1] = 2*100;
		planner_line_to(x, 100, 0, 0, def.acc_default);
		x[1] = -10*100;
		planner_line_to(x, 30, 0, 0, def.acc_default);
	}
	if (rz) {
		int32_t x[3] = {0, 0, -def.size[2]};
		planner_line_to(x, 600, 0, 0, def.acc_default);
		x[2] = 2*100;
		planner_line_to(x, 100, 0, 0, def.acc_default);
		x[2] = -10*100;
		planner_line_to(x, 30, 0, 0, def.acc_default);
	}
	planner_function(set_pos_0);
}

void planner_pre_calculate(void)
{
	int i;
	for (i = 0; i < plan_len; i++)
	{
		if (plan[i].type != ACTION_LINE)
			continue;
		if (plan[i].line.len < 0)
		{
			pre_calculate(&(plan[i].line));
		}
	}
}

