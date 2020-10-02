#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <output/output.h>
#include <control/planner/planner.h>

#define min(a,b) ((a) < (b) ? (a) : (b))

void send_queued(int nid)
{
    char buf[50];
    int q = empty_slots();
    snprintf(buf, sizeof(buf), "queued N:%i Q:%i", nid, q);
    output_control_write(buf, min(strlen(buf), sizeof(buf)));
}

void send_dropped(int nid)
{
    char buf[50];
    int q = empty_slots();
    snprintf(buf, sizeof(buf), "dropped N:%i Q:%i", nid, q);
    output_control_write(buf, min(strlen(buf), sizeof(buf)));
}

void send_started(int nid)
{
    char buf[50];
    int q = empty_slots();
    snprintf(buf, sizeof(buf), "started N:%i Q:%i", nid, q);
    output_control_write(buf, min(strlen(buf), sizeof(buf)));
}

void send_completed(int nid)
{
    char buf[50];
    int q = empty_slots();
    snprintf(buf, sizeof(buf), "completed N:%i Q:%i", nid, q);
    buf[49] = 0;
    output_control_write(buf, min(strlen(buf), sizeof(buf)));
}

void send_completed_with_pos(int nid, const int32_t *pos)
{
    char buf[50];
    int q = empty_slots();
    snprintf(buf, sizeof(buf), "completed N:%i Q:%i X:%ld Y:%ld Z:%ld", nid, q, (long)pos[0], (long)pos[1], (long)pos[2]);
    buf[49] = 0;
    output_control_write(buf, min(strlen(buf), sizeof(buf)));
}

void send_failed(int nid)
{
    char buf[50];
    snprintf(buf, sizeof(buf), "failed N:%i move failed", nid);
    buf[49] = 0;
    output_control_write(buf, min(strlen(buf), sizeof(buf)));
}

void send_ok(int nid)
{
    send_queued(nid);
    send_started(nid);
    send_completed(nid);
}

void send_error(int nid, const char *err)
{
    char buf[50];
    snprintf(buf, sizeof(buf), "error N:%i %s", nid, err);
    buf[49] = 0;
    output_control_write(buf, min(strlen(buf), sizeof(buf)));
}

void send_warning(int nid, const char *err)
{
    char buf[50];
    snprintf(buf, sizeof(buf), "warning N:%i %s", nid, err);
    buf[49] = 0;
    output_control_write(buf, min(strlen(buf), sizeof(buf)));
}

