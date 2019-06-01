#include <stdio.h>
#include <print_status.h>
#include "print_events.h"
#include <shell_print.h>
#include <moves.h>
#include <planner.h>

void print_endstops(int nid)
{
    char buf[128];
    send_started(nid);
    int q = empty_slots();
    cnc_endstops stops = moves_get_endstops();
    snprintf(buf, 128, "completed N: %i Q: %i X: %i Y: %i Z: %i P: %i", nid, q, stops.stop_x, stops.stop_y, stops.stop_z, stops.probe_z);
    shell_send_string(buf);
}

void print_position(int nid)
{
    send_started(nid);
    int q = empty_slots();
    char buf[128];
    snprintf(buf, 128, "completed N: %i Q: %i X: %0.2lf Y: %0.2lf Z: %0.2lf", nid, q, position.pos[0], position.pos[1], position.pos[2]);
    shell_send_string(buf);
}
