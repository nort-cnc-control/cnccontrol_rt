#include <stddef.h>
#include "moves.h"
#include "planner.h"

#define QUEUE_SIZE 20

static steppers_definition def;
extern cnc_position position;

static void (*finish_action)(void);

typedef struct {
	int32_t x[3];
	int32_t feed;
	int32_t feed0;
	int32_t feed1;
} line_plan;

static line_plan plan[QUEUE_SIZE];
static int plan_len;

static void line_started(void)
{
}


static void line_finished(void)
{
	int i;
	for (i = 0; i < plan_len - 1; i++) {
		plan[i] = plan[i + 1];
	}
	plan_len--;

	if (plan_len == 0)
	{
		if (finish_action != NULL)
	       	{
			finish_action();
			finish_action = NULL;
		}
		def.line_finished();
		return;
	}
	if (plan[0].feed > 0)
		set_speed(plan[0].feed);
	move_line_to(plan[0].x, plan[0].feed0, plan[0].feed1);
}

void init_planner(steppers_definition pd)
{
	finish_action = NULL;
	steppers_definition sd = pd;
	def = pd;
	sd.line_started = line_started;
	sd.line_finished = line_finished;
	init_moves(sd);
}

int planner_line_to(int32_t x[3], int feed)
{
	if (plan_len >= QUEUE_SIZE)
		return -1;

	plan[plan_len].x[0] = x[0];
	plan[plan_len].x[1] = x[1];
	plan[plan_len].x[2] = x[2];
	plan[plan_len].feed = feed;
	plan[plan_len].feed0 = 0;
	plan[plan_len].feed1 = 0;
	plan_len++;

	if (plan_len == 1) {
		def.line_started();
		if (plan[0].feed > 0)
			set_speed(plan[0].feed);
		move_line_to(plan[0].x, plan[0].feed0, plan[0].feed1);
	}
	return 0;
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
	finish_action = set_pos_0;
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
}

