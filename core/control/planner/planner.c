#include <string.h>
#include <stddef.h>
#include "moves.h"
#include "planner.h"
#include <math.h>
#include <err.h>

#define QUEUE_SIZE 64

steppers_definition def;
extern cnc_position position;

static volatile int search_begin;

static void (*finish_action)(void);

typedef enum {
	ACTION_LINE = 0,
	ACTION_FUNCTION,
} action_type;

typedef struct {
	int nid;
	int ns;
	int ne;
	action_type type;
	union {
		line_plan line;
		void (*f)(void);
	};
} action_plan;

static action_plan plan[QUEUE_SIZE];
static int plan_cur = 0;
static int plan_last = 0;

static void (*ev_send_started)(int nid);
static void (*ev_send_completed)(int nid);
static void (*ev_send_queued)(int nid);

static void pop_cmd(void)
{
	plan_cur++;
	plan_cur %= QUEUE_SIZE;
}


static void line_started(void)
{
	def.line_started();
}

static void get_cmd(void)
{
	int res;
	action_plan *cp = &plan[plan_cur];

	if (plan_last == plan_cur)
	{
		return;
	}

	if (cp->ns)
		ev_send_started(cp->nid);
	switch (cp->type) {
	case ACTION_LINE:
		res = moves_line_to(&(cp->line));
		if (res == -E_NEXT)
		{
			if (cp->ne)
				ev_send_completed(cp->nid);
			pop_cmd();
			get_cmd();
		}
		break;
	case ACTION_FUNCTION:
		cp->f();
		ev_send_completed(cp->nid);
		pop_cmd();
		get_cmd();
		break;
	}
}

static void line_finished(void)
{
	action_plan *cp = &plan[plan_cur];
	if (cp->ne)
		ev_send_completed(cp->nid);
	def.line_finished();
	pop_cmd();
	get_cmd();
}

static void line_error(void)
{
	line_finished();
}


void init_planner(steppers_definition pd,
                  void (*arg_send_queued)(int nid),
                  void (*arg_send_started)(int nid),
                  void (*arg_send_completed)(int nid))
{
    ev_send_started = arg_send_started;
    ev_send_completed = arg_send_completed;
    ev_send_queued = arg_send_queued;
	plan_cur = plan_last = 0;
	search_begin = 0;
	finish_action = NULL;
	
	def = pd;
	def.feed_max = FIXED_ENCODE(pd.feed_max);
	def.feed_base = FIXED_ENCODE(pd.feed_base);

	steppers_definition sd = def;
	sd.line_started = line_started;
	sd.line_error = line_error;
	sd.line_finished = line_finished;
	moves_init(sd);
}

int used_slots(void)
{
	int plan_len = plan_last - plan_cur;
	if (plan_len < 0)
		plan_len += QUEUE_SIZE;
	return plan_len;
}

int empty_slots(void)
{
	return QUEUE_SIZE - used_slots() - 1;
}

static int break_on_endstops(int32_t *dx, void *user_data)
{
	cnc_endstops endstops = def.get_endstops();
	if (endstops.stop_x && dx[0] < 0)
		return 1;

	if (endstops.stop_y && dx[1] < 0)
		return 1;

	if (endstops.stop_z && dx[2] < 0)
		return 1;

	return 0;
}

static int _planner_line_to(fixed x[3], int (*cbr)(int32_t *, void *), void *usr_data,
                    int32_t feed, int32_t f0, int32_t f1, int32_t acc, int nid, int ns, int ne)
{
	action_plan *cur;
	if (empty_slots() == 0)
		return -E_NOMEM;

	if (x[0] == 0 && x[1] == 0 && x[2] == 0)
		return empty_slots();

	cur = &plan[plan_last];
	cur->type = ACTION_LINE;
	cur->nid = nid;
	cur->ne = ne;
	cur->ns = ns;
    if (cbr != NULL)
    {
        cur->line.check_break = cbr;
        cur->line.check_break_data = usr_data;
    }
    else
    {
		cur->line.check_break = break_on_endstops;
        cur->line.check_break_data = x;
    }
	cur->line.x[0] = x[0];
	cur->line.x[1] = x[1];
	cur->line.x[2] = x[2];
	cur->line.feed = FIXED_ENCODE(feed);
	cur->line.feed0 = FIXED_ENCODE(f0);
	cur->line.feed1 = FIXED_ENCODE(f1);
	cur->line.acceleration = acc;
	cur->line.len = -1;
	cur->line.acc_steps = -1;
	cur->line.dec_steps = -1;
	plan_last++;
	plan_last %= QUEUE_SIZE;
    return empty_slots();
}

int planner_line_to(fixed x[3], int32_t feed, int32_t f0, int32_t f1, int32_t acc, int nid)
{
    if (empty_slots() == 0)
    {
        return -E_NOMEM;
    }

	_planner_line_to(x, NULL, NULL, feed, f0, f1, acc, nid, 1, 1);
	ev_send_queued(nid);
	if (used_slots() == 1) {
		get_cmd();
	}
	return empty_slots();
}

static void _planner_function(void (*f)(void), int nid, int ns, int ne)
{
	action_plan *cur;
	if (empty_slots() == 0)
		return;

	cur = &plan[plan_last];
	cur->nid = nid;
	cur->ne = ne;
	cur->ns = ns;
	cur->type = ACTION_FUNCTION;
	cur->f = f;
	plan_last++;
	plan_last %= QUEUE_SIZE;
}

int planner_function(void (*f)(void), int nid)
{
	_planner_function(f, nid, 1, 1);
	ev_send_queued(nid);
	if (used_slots() == 1) {
		get_cmd();
	}
	return empty_slots();
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

static int break_on_probe(int32_t *dx, void *user_data)
{
	cnc_endstops endstops = def.get_endstops();
	if (endstops.probe_z && dx[2] > 0)
		return 1;

	return 0;
}

void planner_z_probe(int nid)
{
	fixed x[3] = {0, 0, FIXED_ENCODE(def.size[2])};
	_planner_line_to(x, break_on_probe, NULL, def.probe_travel, 0, 0, def.acc_default, nid, 1, 0);
	x[2] = FIXED_ENCODE(-1);
	_planner_line_to(x, NULL, NULL, def.probe_travel, 0, 0, def.acc_default, nid, 0, 0);
	x[2] = FIXED_ENCODE(2);
	_planner_line_to(x, break_on_probe, NULL, def.probe_precise, 0, 0, def.acc_default, nid, 0, 1);

	ev_send_queued(nid);
	if (used_slots() == 3) {
		get_cmd();
	}
}

void planner_find_begin(int rx, int ry, int rz, int nid)
{
	int s = (used_slots() == 0);
	int f = 1;
	srx = rx;
	sry = ry;
	srz = rz;
	if (rx) {
		fixed x[3] = {-FIXED_ENCODE(def.size[0]), 0, 0};
		_planner_line_to(x, NULL, NULL, def.es_travel, 0, 0, def.acc_default, nid, 1, 0);
		x[0] = FIXED_ENCODE(2);
		_planner_line_to(x, NULL, NULL, def.es_travel, 0, 0, def.acc_default, nid, 0, 0);
		x[0] = FIXED_ENCODE(-10);
		_planner_line_to(x, NULL, NULL, def.es_precise, 0, 0, def.acc_default, nid, 0, 0);
		f = 0;
	}
	if (ry) {
		fixed x[3] = {0, -FIXED_ENCODE(def.size[1]), 0};
		_planner_line_to(x, NULL, NULL, def.es_travel, 0, 0, def.acc_default, nid, f, 0);
		x[1] = FIXED_ENCODE(2);
		_planner_line_to(x, NULL, NULL, def.es_travel, 0, 0, def.acc_default, nid, 0, 0);
		x[1] = FIXED_ENCODE(-10);
		_planner_line_to(x, NULL, NULL, def.es_precise, 0, 0, def.acc_default, nid, 0, 0);
		f = 0;
	}
	if (rz) {
		fixed x[3] = {0, 0, -FIXED_ENCODE(def.size[2])};
		_planner_line_to(x, NULL, NULL, def.es_travel, 0, 0, def.acc_default, nid, f, 0);
		x[2] = FIXED_ENCODE(2);
		_planner_line_to(x, NULL, NULL, def.es_travel, 0, 0, def.acc_default, nid, 0, 0);
		x[2] = FIXED_ENCODE(-10);
		_planner_line_to(x, NULL, NULL, def.es_precise, 0, 0, def.acc_default, nid, 0, 0);
	}
	_planner_function(set_pos_0, nid, 0, 1);

	ev_send_queued(nid);
	if (s)
		get_cmd();
}

void planner_pre_calculate(void)
{
	int i;
	for (i = 0; i < used_slots(); i++)
	{
		int pos = (plan_cur + i) % QUEUE_SIZE;
		action_plan *p = &plan[pos];
		if (p->type != ACTION_LINE)
			continue;
		if (p->line.len < 0)
		{
			line_pre_calculate(&(p->line));
		}
	}
}

