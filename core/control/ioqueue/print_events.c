#include <stdio.h>
#include <unistd.h>
#include <planner.h>
#include <output.h>
#include <string.h>

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
    output_control_write(buf, min(strlen(buf), sizeof(buf)));
}
