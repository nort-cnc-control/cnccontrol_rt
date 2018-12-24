
#include "control.h"
#include <planner.h>
#include <print_events.h>

static void cb_send_queued(int nid)
{
    send_queued(nid);
}

static void cb_send_started(int nid)
{
    send_started(nid);
}

static void cb_send_completed(int nid)
{
    send_completed(nid);
}

void init_control(steppers_definition pd)
{
    init_planner(pd, cb_send_queued, cb_send_started, cb_send_completed);
}
