#include <stdio.h>
#include <shell_print.h>
#include <planner.h>

void send_queued(int nid)
{
    char buf[128];
    int q = empty_slots();
    snprintf(buf, 128, "queued N: %i Q: %i", nid, q);
    shell_send_string(buf);
}

void send_dropped(int nid)
{
    char buf[128];
    int q = empty_slots();
    snprintf(buf, 128, "dropped N: %i Q: %i", nid, q);
    shell_send_string(buf);
}

void send_started(int nid)
{
    char buf[128];
    int q = empty_slots();
    snprintf(buf, 128, "started N: %i Q: %i", nid, q);
    shell_send_string(buf);
}

void send_completed(int nid)
{
    char buf[128];
    int q = empty_slots();
    snprintf(buf, 128, "completed N: %i Q: %i", nid, q);
    shell_send_string(buf);
}

void send_ok(int nid)
{
    send_queued(nid);
    send_started(nid);
    send_completed(nid);
}

void send_error(int nid, const char *err)
{
    char buf[128];
    snprintf(buf, 128, "error N: %i %s", nid, err);
    buf[127] = 0;
    shell_send_string(buf);
}
