#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <control/commands/status/print_status.h>
#include <control/ioqueue/print_events.h>
#include <control/moves/moves.h>
#include <control/planner/planner.h>
#include <output/output.h>

#define min(a,b) ((a) < (b) ? (a) : (b))

void print_endstops(int nid)
{
    char buf[128]; 
    send_started(nid);
    int q = empty_slots();
    cnc_endstops stops = moves_get_endstops();
    snprintf(buf, sizeof(buf), "completed N:%i Q:%i EX:%i EY:%i EZ:%i EP:%i", nid, q, stops.stop_x, stops.stop_y, stops.stop_z, stops.probe);
    buf[127] = 0;
    output_control_write(buf, min(strlen(buf), sizeof(buf)));
}

void print_position(int nid)
{
    char buf[128];
    send_started(nid);
    int q = empty_slots();

    snprintf(buf, sizeof(buf), "completed N:%i Q:%i X:%ld Y:%ld Z:%ld", nid, q, (long)position.pos[0], (long)position.pos[1], (long)position.pos[2]);
    buf[127] = 0;
    output_control_write(buf, min(strlen(buf), sizeof(buf)));
}

