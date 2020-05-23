#include <stdio.h>
#include <print_status.h>
#include "print_events.h"
#include <moves.h>
#include <planner.h>
#include <output.h>
#include <unistd.h>
#include <string.h>

#define min(a,b) ((a) < (b) ? (a) : (b))

void print_endstops(int nid)
{
    char buf[128]; 
    send_started(nid);
    int q = empty_slots();
    cnc_endstops stops = moves_get_endstops();
    snprintf(buf, sizeof(buf), "completed N:%i Q:%i EX:%i EY:%i EZ:%i EP:%i", nid, q, stops.stop_x, stops.stop_y, stops.stop_z, stops.probe);
    output_control_write(buf, min(strlen(buf), sizeof(buf)));
}

void print_position(int nid)
{
    char buf[128];
    send_started(nid);
    int q = empty_slots();

    int32_t x, y, z;
    x = position.pos[0];
    y = position.pos[1];
    z = position.pos[2];

    snprintf(buf, sizeof(buf), "completed N:%i Q:%i X:%i Y:%i Z:%i", nid, q, x, y, z);
    output_control_write(buf, min(strlen(buf), sizeof(buf)));
}

