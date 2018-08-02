#include <stddef.h>
#include "moves.h"
#include "planner.h"
#include <shell/shell.h>

#define QUEUE_SIZE 50

static steppers_definition def;
extern cnc_position position;

static volatile int search_begin;

static void (*finish_action)(void);

typedef enum {
	ACTION_LINE = 0,
	ACTION_FUNCTION,
} action_type;

typedef struct {
	int32_t x[3];
	int32_t feed;
	int32_t feed0;
	int32_t feed1;
} line_plan;

typedef struct {
	action_type type;
	line_plan line;
	void (*f)(void);
} action_plan;

static action_plan plan[QUEUE_SIZE];
static int plan_len;

static void line_started(void)
{
}

static void pop_cmd(void)
{
	int i;
	for (i = 0; i < plan_len - 1; i++) {
		plan[i] = plan[i + 1];
	}
	plan_len--;
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
		if (plan[0].line.feed > 0)
			set_speed(plan[0].line.feed);
		move_line_to(plan[0].line.x, plan[0].line.feed0, plan[0].line.feed1);
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


int planner_line_to(int32_t x[3], int feed)
{
	if (plan_len >= QUEUE_SIZE)
		return -1;

	plan[plan_len].type = ACTION_LINE;
	plan[plan_len].line.x[0] = x[0];
	plan[plan_len].line.x[1] = x[1];
	plan[plan_len].line.x[2] = x[2];
	plan[plan_len].line.feed = feed;
	plan[plan_len].line.feed0 = 0;
	plan[plan_len].line.feed1 = 0;
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
	if (srx && sry && srz)
		position.position_error = 0;
	search_begin = 0;
}

void planner_find_begin(int rx, int ry, int rz)
{
	srx = rx;
	sry = ry;
	srz = rz;
	search_begin = 1;
	if (rx) {
		int32_t x[3] = {-def.size[0], 0, 0};
		planner_line_to(x, 600);
		x[0] = 2*100;
		planner_line_to(x, 100);
		x[0] = -10*100;
		planner_line_to(x, 30);
	}
	if (ry) {
		int32_t x[3] = {0, -def.size[1], 0};
		planner_line_to(x, 600);
		x[1] = 2*100;
		planner_line_to(x, 100);
		x[1] = -10*100;
		planner_line_to(x, 30);
	}
	if (rz) {
		int32_t x[3] = {0, 0, -def.size[2]};
		planner_line_to(x, 600);
		x[2] = 2*100;
		planner_line_to(x, 100);
		x[2] = -10*100;
		planner_line_to(x, 30);
	}
	planner_function(set_pos_0);
}

