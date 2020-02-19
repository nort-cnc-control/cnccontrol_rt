#include <string.h>
#include <stddef.h>
#include <stdbool.h>
#include "moves.h"
#include "planner.h"
#include <math.h>
#include <err.h>
#include <stdlib.h>
#include <stdio.h>

#define QUEUE_SIZE 10

steppers_definition def;
extern cnc_position position;

static volatile int search_begin;

static volatile bool locked = true;
static volatile bool fail_on_endstops = true;

static void (*finish_action)(void);
static void _planner_lock(void);

typedef enum {
    ACTION_NONE = 0,
    ACTION_LINE,
    ACTION_ARC,
} action_type;

typedef struct {
    int nid;
    int notify_start;
    int notify_end;
    action_type type;
    union {
        line_plan line;
        arc_plan arc;
    };
} action_plan;

static action_plan plan[QUEUE_SIZE];
static int plan_cur = 0;
static int plan_last = 0;
static int plan_len = 0;

static bool break_on_probe = false;

static void (*ev_send_started)(int nid);
static void (*ev_send_completed)(int nid);
static void (*ev_send_queued)(int nid);
static void (*ev_send_dropped)(int nid);
static void (*ev_send_failed)(int nid);

static void pop_cmd(void)
{
    memset(&plan[plan_cur], 0, sizeof(action_plan));
    if (plan_len > 0)
    {
        plan_cur = (plan_cur + 1) % QUEUE_SIZE;
        plan_len--;
    }
}

static void line_started(void)
{
    if (locked)
        return;
    def.line_started();
}

static void get_cmd(void)
{
    if (plan_len == 0)
    {
    	return;
    }

    action_plan *cp = &plan[plan_cur];
    int res;
    
    if (cp->notify_start)
        ev_send_started(cp->nid);

    switch (cp->type) {
    case ACTION_LINE:
        res = moves_line_to(&(cp->line));
        if (res == -E_NEXT)
        {
            if (cp->notify_end)
                ev_send_completed(cp->nid);
            pop_cmd();
            get_cmd();
        }
        break;
    case ACTION_ARC:
        res = moves_arc_to(&(cp->arc));
        if (res == -E_NEXT)
        {
            if (cp->notify_end)
                ev_send_completed(cp->nid);
            pop_cmd();
            get_cmd();
        }
        break;
    case ACTION_NONE:
        pop_cmd();
        get_cmd();
        break;
    }
}

static void line_finished(void)
{
    action_plan *cp = &plan[plan_cur];
    if (cp->notify_end)
        ev_send_completed(cp->nid);
    def.line_finished();
    pop_cmd();

    if (locked)
        return;
    
    get_cmd();
}

static void endstops_touched(void)
{
    if (!fail_on_endstops)
    {
        line_finished();
    }
    else
    {
        action_plan *cp = &plan[plan_cur];
	_planner_lock();
        def.line_error();
        ev_send_failed(cp->nid);
        pop_cmd();
    }
}

void init_planner(steppers_definition pd,
                  void (*arg_send_queued)(int nid),
                  void (*arg_send_started)(int nid),
                  void (*arg_send_completed)(int nid),
                  void (*arg_send_dropped)(int nid),
                  void (*arg_send_failed)(int nid))
{
    ev_send_started = arg_send_started;
    ev_send_completed = arg_send_completed;
    ev_send_queued = arg_send_queued;
    ev_send_dropped = arg_send_dropped;
    ev_send_failed = arg_send_failed;
    plan_cur = plan_last = 0;
    plan_len = 0;
    search_begin = 0;
    finish_action = NULL;

    def = pd;
    def.feed_max = pd.feed_max;
    def.feed_base = pd.feed_base;

    steppers_definition sd = def;
    sd.line_started = line_started;
    sd.endstops_touched = endstops_touched;
    sd.line_finished = line_finished;
    moves_init(sd);
}

int used_slots(void)
{
    return plan_len;
}

int empty_slots(void)
{
    return QUEUE_SIZE - used_slots();
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

    if (break_on_probe && endstops.probe && dx[2] >= 0)
        return 1;

    return 0;
}

static int _planner_line_to(double x[3], int (*cbr)(int32_t *, void *), void *usr_data,
                            double feed, double f0, double f1, int32_t acc, int nid, int ns, int ne)
{
    action_plan *cur;

    if (x[0] == 0 && x[1] == 0 && x[2] == 0)
        return 0;

    cur = &plan[plan_last];
    cur->type = ACTION_LINE;
    cur->nid = nid;
    cur->notify_end = ne;
    cur->notify_start = ns;
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
    cur->line.feed = feed;
    cur->line.feed0 = f0;
    cur->line.feed1 = f1;
    cur->line.acceleration = acc;
    cur->line.len = -1;
    cur->line.acc_steps = -1;
    cur->line.dec_steps = -1;

    plan_last = (plan_last + 1) % QUEUE_SIZE;
    plan_len++;
    return 1;
}

int planner_line_to(double x[3], double feed, double f0, double f1, int32_t acc, int nid)
{
    if (planner_is_locked())
    {
        return -E_LOCKED;
    }

    if (empty_slots() == 0)
    {
        return -E_NOMEM;
    }

    int res = _planner_line_to(x, NULL, NULL, feed, f0, f1, acc, nid, 1, 1);
    if (res)
    {
        ev_send_queued(nid);
        if (used_slots() == 1) {
            get_cmd();
        }
    }
    else
    {
        ev_send_dropped(nid);
    }
    return empty_slots();
}

static int _planner_arc_to(double x[3], double d, arc_plane plane, int cw, int (*cbr)(int32_t *, void *), void *usr_data,
                           double feed, double f0, double f1, int32_t acc, int nid, int ns, int ne)
{
    action_plan *cur;

    if (x[0] == 0 && x[1] == 0 && x[2] == 0)
        return 0;

    cur = &plan[plan_last];
    cur->type = ACTION_ARC;
    cur->nid = nid;
    cur->notify_end = ne;
    cur->notify_start = ns;
    if (cbr != NULL)
    {
        cur->arc.check_break = cbr;
        cur->arc.check_break_data = usr_data;
    }
    else
    {
        cur->arc.check_break = break_on_endstops;
        cur->arc.check_break_data = x;
    }
    cur->arc.x[0] = x[0];
    cur->arc.x[1] = x[1];
    cur->arc.x[2] = x[2];
    cur->arc.d = d;
    cur->arc.cw = cw;
    cur->arc.plane = plane;
    cur->arc.feed = feed;
    cur->arc.feed0 = f0;
    cur->arc.feed1 = f1;
    cur->arc.acceleration = acc;
    cur->arc.ready = 0;
    cur->arc.acc_steps = -1;
    cur->arc.dec_steps = -1;

    plan_last = (plan_last + 1) % QUEUE_SIZE;
    plan_len++;
    return 1;
}


int planner_arc_to(double x[3], double d, arc_plane plane, int cw, double feed, double f0, double f1, int32_t acc, int nid)
{
    if (planner_is_locked())
    {
        return -E_LOCKED;
    }

    if (empty_slots() == 0)
    {
        return -E_NOMEM;
    }

    int res = _planner_arc_to(x, d, plane, cw, NULL, NULL, feed, f0, f1, acc, nid, 1, 1);
    if (res)
    {
        ev_send_queued(nid);
        if (used_slots() == 1) {
            get_cmd();
        }
    }
    else
    {
        ev_send_dropped(nid);
    }
    return empty_slots();
}

static int srx, sry, srz;

void enable_break_on_probe(bool en)
{
    break_on_probe = en;
}

void planner_pre_calculate(void)
{
    int i;
    for (i = 0; i < used_slots(); i++)
    {
        int pos = (plan_cur + i) % QUEUE_SIZE;
        action_plan *p = &plan[pos];
        switch(p->type)
        {
            case ACTION_LINE:
                if (p->line.len < 0)
                {
                    line_pre_calculate(&(p->line));
                }
                break;
            case ACTION_ARC:
                if (p->arc.ready == 0)
                {
                    arc_pre_calculate(&(p->arc));
                }
                break;
            default:
                break;
        }
    }
}

static void _planner_lock(void)
{
    locked = 1;
    plan_cur = plan_last = 0;
    plan_len = 0;
    moves_break();
}

void planner_lock(void)
{
    _planner_lock();
    def.line_finished();
}

void planner_unlock(void)
{
    locked = 0;
}

int planner_is_locked(void)
{
    return locked;
}

void planner_fail_on_endstops(bool fail)
{
    fail_on_endstops = fail;
}
