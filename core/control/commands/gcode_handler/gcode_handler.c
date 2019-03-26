#include <stddef.h>
#include <err.h>
#include <gcodes.h>
#include <print_events.h>
#include <print_status.h>
#include <planner.h>
#include <shell.h>
#include <print.h>
#include <serial_sender.h>

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
            int32_t acc = def.acc_default;
            double x[3] = {0, 0, 0};
            for (i = 1; i < ncmds; i++) {
                switch (cmds[i].type) {
                case 'X':
                    x[0] = cmds[i].val_f/100.0;
                    break;
                case 'Y':
                    x[1] = cmds[i].val_f/100.0;
                    break;
                case 'Z':
                    x[2] = cmds[i].val_f/100.0;
                    break;
                case 'F':
                    f = cmds[i].val_i;
                    break;
                case 'P':
                    feed0 = cmds[i].val_i;
                    break;
                case 'L':
                    feed1 = cmds[i].val_i;
                    break;
                case 'T':
                    acc = cmds[i].val_i;
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
                return res;
            }
            else
            {
                send_error(nid, "problem with planning line");
                return res;
            }
            break;
        }
        case 2:
        case 3: {
            int i;
            double f = 0, feed0 = 0, feed1 = 0;
	        int32_t acc = def.acc_default;
            double x[3] = {0, 0, 0};
            int plane = XY;
            double d = 0;
            for (i = 1; i < ncmds; i++) {
                switch (cmds[i].type) {
                case 'X':
                    x[0] = cmds[i].val_f/100.0;
                    break;
                case 'Y':
                    x[1] = cmds[i].val_f/100.0;
                    break;
                case 'Z':
                    x[2] = cmds[i].val_f/100.0;
                    break;
                case 'D':
                    d = cmds[i].val_f/100.0;
                    break;
                case 'F':
                    f = cmds[i].val_i;
                    break;
                case 'P':
                    feed0 = cmds[i].val_i;
                    break;
                case 'L':
                    feed1 = cmds[i].val_i;
                    break;
                case 'T':
                    acc = cmds[i].val_i;
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
            int res = planner_arc_to(x, d, plane, cw, f, feed0, feed1, acc, nid);
            if (res >= 0)
            {
                return -E_OK;
            }
            else if (res == -E_NOMEM)
            {
                send_error(nid, "no space in buffer");
                return res;
            }
            else
            {
                send_error(nid, "problem with planning arc");
                return res;
            }
            break;
        }

        default:
            send_error(nid, "unknown command");
            return -E_INCORRECT;
        }
        break;
    case 'M':
        switch (cmds[0].val_i) {
        case 114:
            print_position(nid);
            return -E_OK;
        case 119:
            print_endstops(nid);
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
            double x[3] = {0};
            moves_set_position(x);
            moves_reset();
            send_ok(nid);
            return -E_OK;
        }
        case 998:
            moves_reset();
            send_ok(nid);
            return -E_OK;
        case 999:
            def.reboot();
        default:
            send_error(nid, "unknown command");
            return -E_INCORRECT;
        }
        break;
    default:
        send_error(nid, "unknown command");
        return -E_INCORRECT;
    }
    return -E_INCORRECT;
}

int execute_g_command(const char *command)
{
    gcode_frame_t frame;
    int rc;

    if ((rc = parse_cmdline(command, &frame)) < 0) {
        send_error(-1, "parse error");
        shell_send_string("error: ");
        shell_send_string(command);
        shell_send_string("\n\r");
        return rc;
    }

    return handle_g_command(&frame);
}
