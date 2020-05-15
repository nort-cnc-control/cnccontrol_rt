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

static int decimal4(_Decimal64 x)
{
    if (x < 0)
        x = -x;
    return (int)(x * 10000) - ((int)x)*10000;
}

void double2fixed(_Decimal64 x, int *xh, int *xl, char *sign)
{
    if (x < 0)
    {
        x = -x;
        *sign = '-';
    }
    else
    {
        *sign = '+';
    }
    *xh = (int)(x);
    *xl = decimal4(x);
}

void print_position(int nid)
{
    char buf[128];
    send_started(nid);
    int q = empty_slots();

    int xh, yh, zh;
    int xl, yl, zl;
    char xs, ys, zs;
    double2fixed(position.pos[0], &xh, &xl, &xs);
    double2fixed(position.pos[1], &yh, &yl, &ys);
    double2fixed(position.pos[2], &zh, &zl, &zs);

    snprintf(buf, sizeof(buf), "completed N:%i Q:%i X:%c%i.%04i Y:%c%i.%04i Z:%c%i.%04i", nid, q, xs, xh, xl, ys, yh, yl, zs, zh, zl);
    output_control_write(buf, min(strlen(buf), sizeof(buf)));
}

