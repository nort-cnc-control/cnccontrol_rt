
#include "print_status.h"
#include "print_events.h"
#include <shell.h>
#include <print.h>
#include <moves.h>
#include <planner.h>

void print_endstops(int nid)
{
    send_started(nid);
    int q = empty_slots();
    cnc_endstops stops = moves_get_endstops();
    shell_send_string("completed");
    shell_send_string(" N:");
    shell_print_dec(nid);
    shell_send_string(" Q:");
    shell_print_dec(q);
    shell_send_string(" X:");
    shell_send_char(stops.stop_x + '0');
    shell_send_string(" Y:");
    shell_send_char(stops.stop_y + '0');
    shell_send_string(" Z:");
    shell_send_char(stops.stop_z + '0');
    shell_send_string(" P:");
    shell_send_char(stops.probe_z + '0');
    shell_send_string("\r\n");
}

void print_position(int nid)
{
    send_started(nid);
    int q = empty_slots();
    shell_send_string("completed");
    shell_send_string(" N:");
    shell_print_dec(nid);
    shell_send_string(" Q:");
    shell_print_dec(q);
    shell_send_string(" X:");
    shell_print_fixed_2(position.pos[0]);
    shell_send_string(" Y:");
    shell_print_fixed_2(position.pos[1]);
    shell_send_string(" Z:");
    shell_print_fixed_2(position.pos[2]);
    shell_send_string("\r\n");
}
