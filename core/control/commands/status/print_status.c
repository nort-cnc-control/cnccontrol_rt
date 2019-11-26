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

static int decimal4(double x)
{
    if (x < 0)
        x = -x;
    return (int)(x * 10000) - ((int)x)*10000;
}

void print_position(int nid)
{
    char buf[128];
    send_started(nid);
    int q = empty_slots();

    int xh = position.pos[0];
    int xl = decimal4(position.pos[0]);

    int yh = position.pos[1];
    int yl = decimal4(position.pos[1]);

    int zh = position.pos[2];
    int zl = decimal4(position.pos[2]);

    snprintf(buf, sizeof(buf), "completed N:%i Q:%i X:%i.%04i Y:%i.%04i Z:%i.%04i", nid, q, xh, xl, yh, yl, zh, zl);
    output_control_write(buf, min(strlen(buf), sizeof(buf)));
}
