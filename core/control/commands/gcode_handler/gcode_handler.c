#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>

#include <defs.h>

#include <err/err.h>
#include <gcode/gcodes.h>
#include <control/ioqueue/print_events.h>
#include <control/commands/status/print_status.h>
#include <control/planner/planner.h>
#include <control/system.h>

static int handle_g_command(gcode_frame_t *frame)
{
    gcode_cmd_t *cmds = frame->cmds;
    int ncmds = frame->num;
    int nid = -1;

    // skip line number(s)
    while (ncmds > 0 && cmds[0].type == 'N') {
        nid = cmds[0].val_i;
        ncmds--;
        cmds++;
    }

    if (nid == -1)
    {
        send_error(-1, "No command number specified");
        planner_lock();
        return -E_INCORRECT;
    }
    if (ncmds == 0)
    {
        send_ok(nid);
        return -E_OK;
    }
    // parse command line
    switch (cmds[0].type) {
    case 'G':
        switch (cmds[0].val_i) {
        case 0:
        case 1: {
            int i;
            double f = 0, feed0 = 0, feed1 = 0;
            double acc = 0;
            int32_t x[3] = {0, 0, 0};
            for (i = 1; i < ncmds; i++) {
                switch (cmds[i].type) {
                case 'X':
                    x[0] = cmds[i].val_i;
                    break;
                case 'Y':
                    x[1] = cmds[i].val_i;
                    break;
                case 'Z':
                    x[2] = cmds[i].val_i;
                    break;
                case 'F':
                    f = cmds[i].val_f;
                    break;
                case 'P':
                    feed0 = cmds[i].val_f;
                    break;
                case 'L':
                    feed1 = cmds[i].val_f;
                    break;
                case 'T':
                    acc = cmds[i].val_f;
                    break;
                }
            }
            int res = planner_line_to(x, f, feed0, feed1, acc, nid);
            if (res >= 0)
            {
                return -E_OK;
            }
            else if (res == -E_NOMEM)
            {
                send_error(nid, "no space in buffer");
                planner_lock();
                return res;
            }
            else if (res == -E_LOCKED)
            {
                send_error(nid, "system is locked");
                return res;
            }
            else
            {
                send_error(nid, "problem with planning line");
                planner_lock();
                return res;
            }
            break;
        }
        case 2:
        case 3: {
            int i;
            double f = 0, feed0 = 0, feed1 = 0;
            int32_t x1[2] = {0, 0};
            int32_t x2[2] = {0, 0};
            int32_t h = 0;
            double a = 0, b = 0, len = 0;

	    double acc = 0;
            int plane = XY;
            for (i = 1; i < ncmds; i++) {
                switch (cmds[i].type) {
                case 'X':
                    x2[0] = cmds[i].val_i;
                    break;
                case 'Y':
                    x2[1] = cmds[i].val_i;
                    break;
                case 'R':
                    x1[0] = cmds[i].val_i;
                    break;
                case 'S':
                    x1[1] = cmds[i].val_i;
                    break;
                case 'H':
                    h = cmds[i].val_i;
                    break;
                case 'D':
                    len = cmds[i].val_f;
                    break;
                case 'A':
                    a = cmds[i].val_f;
                    break;
                case 'B':
                    b = cmds[i].val_f;
                    break;
                case 'F':
                    f = cmds[i].val_f;
                    break;
                case 'P':
                    feed0 = cmds[i].val_f;
                    break;
                case 'L':
                    feed1 = cmds[i].val_f;
                    break;
                case 'T':
                    acc = cmds[i].val_f;
                    break;
                case 'G':
                    switch (cmds[i].val_i)
                    {
                    case 17:
                        plane = XY;
                        break;
                    case 18:
                        plane = YZ;
                        break;
                    case 19:
                        plane = ZX;
                        break;
                    default:
                        break;
                    }
                    break;
                }
            }
            int cw = (cmds[0].val_i == 2);
            int res = planner_arc_to(x1, x2, h, len, a, b, plane, cw, f, feed0, feed1, acc, nid);
            if (res >= 0)
            {
                return -E_OK;
            }
            else if (res == -E_NOMEM)
            {
                send_error(nid, "no space in buffer");
                planner_lock();
                return res;
            }
            else if (res == -E_LOCKED)
            {
                send_error(nid, "system is locked");
                return res;
            }
            else
            {
                send_error(nid, "problem with planning arc");
                planner_lock();
                return res;
            }
            break;
        }
        default:
        {
            char buf[60];
            snprintf(buf, 60, "unknown command G%i", cmds[0].val_i);
            send_error(nid, buf);
            planner_lock();
            return -E_INCORRECT;
        }
        }
        break;
    case 'M':
        switch (cmds[0].val_i) {
        case 3:
        case 5:
        {
            int tool = 0;
            int on = (cmds[0].val_i == 3);
	    int i;
            for (i = 1; i < ncmds; i++) {
                switch (cmds[i].type) {
                case 'D':
                    tool = cmds[i].val_i;
		    break;
                }
	    }
            int res = planner_tool(tool, on, nid);
            if (res >= 0)
            {
                return -E_OK;
            }
            else if (res == -E_NOMEM)
            {
                send_error(nid, "no space in buffer");
                planner_lock();
                return res;
            }
            else if (res == -E_LOCKED)
            {
                send_error(nid, "system is locked");
                return res;
            }
            else
            {
                send_error(nid, "problem with planning tool");
                planner_lock();
                return res;
            }

            return -E_OK;
	}
        case 100: {
            int i;
            steppers_definition def = moves_common_def;
            for (i = 1; i < ncmds; i++) {
                switch (cmds[i].type) {
                case 'X':
                    def.steps_per_unit[0] = cmds[i].val_f;
                    break;
                case 'Y':
                    def.steps_per_unit[1] = cmds[i].val_f;
                    break;
                case 'Z':
                    def.steps_per_unit[2] = cmds[i].val_f;
                    break;
                case 'F':
                    def.feed_max = cmds[i].val_f;
                    break;
                case 'A':
                    def.acc_default = cmds[i].val_f;
                    break;
                case 'B':
                    def.feed_base = cmds[i].val_f;
                    break;
                }
            }

            if (def.steps_per_unit[0] > 0 &&
                def.steps_per_unit[1] > 0 &&
                def.steps_per_unit[2] > 0 &&
                def.feed_max > 0 &&
                def.feed_base > 0 &&
                def.acc_default > 0)
            {
                def.configured = true;
            }
            moves_common_init(&def);
            send_ok(nid);
            return -E_OK;
        }
        case 114:
            send_queued(nid);
            print_position(nid);
            return -E_OK;
        case 119:
            send_queued(nid);
            print_endstops(nid);
            return -E_OK;
        case 800:
            planner_unlock();
            send_ok(nid);
            return -E_OK;
        case 801:
            planner_lock();
            send_ok(nid);
            return -E_OK;
        case 802:
            planner_fail_on_endstops(false);
            send_ok(nid);
            return -E_OK;
        case 803:
            planner_fail_on_endstops(true);
            send_ok(nid);
            return -E_OK;
	case 995:
            enable_break_on_probe(false);
            send_ok(nid);
            return -E_OK;
        case 996:
            enable_break_on_probe(true);
            send_ok(nid);
            return -E_OK;
        case 997: {
            int32_t x[3] = {0};
            moves_common_set_position(x);
            send_ok(nid);
            return -E_OK;
        }
        case 999:
            system_reboot();
            // for debug cases
            send_ok(nid);
            return -E_OK;
        default:
        {
            char buf[60];
            snprintf(buf, 60, "unknown command M%i", cmds[0].val_i);
            send_error(nid, buf);
            planner_lock();
            return -E_INCORRECT;
        }
        }
        break;
    default:
    {
        char buf[60];
        snprintf(buf, 60, "unknown command %c%i", cmds[0].type, cmds[0].val_i);
        send_error(nid, buf);
        planner_lock();
        return -E_INCORRECT;
    }
    }
    planner_lock();
    return -E_INCORRECT;
}

int execute_g_command(const unsigned char *command, ssize_t len)
{
    gcode_frame_t frame;
    int rc;

    if (len < 0)
        len = strlen((const char *)command);

    rc = parse_cmdline(command, len, &frame);
    switch (rc)
    {
        case -E_CRC:
            send_error(-1, "CRC error");
            return rc;
        case -E_OK:
            return handle_g_command(&frame);
        default:
        {
            planner_lock();
            char buf[160];
            snprintf(buf, sizeof(buf), "parse error: %i [", (int)len);
            int i;
            for (i = 0; i < len && strlen(buf) < 160-2; i++)
            {
                char cbuf[10];
                sprintf(cbuf, "%02x ", command[i]);
                strcat(buf, cbuf);
            }
            strcat(buf, "]");
            buf[159] = 0;
            send_error(-1, buf);
            return rc;
        }
    }
}
