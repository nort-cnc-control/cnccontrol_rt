
#include <control/control.h>
#include <control/system.h>
#include <control/planner/planner.h>
#include <control/ioqueue/print_events.h>

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

static void cb_send_dropped(int nid)
{
    send_dropped(nid);
}

static void cb_send_failed(int nid)
{
    send_failed(nid);
}

static void cb_send_completed_with_pos(int nid, const int32_t *pos)
{
    send_completed_with_pos(nid, pos);
}

void init_control(steppers_definition *pd, gpio_definition *gd)
{
    init_planner(pd, gd, cb_send_queued, cb_send_started, cb_send_completed, cb_send_completed_with_pos, cb_send_dropped, cb_send_failed);
    system_init(pd->reboot);
}

