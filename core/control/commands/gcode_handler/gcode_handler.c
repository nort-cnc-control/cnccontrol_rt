#include <stddef.h>
#include <err.h>
#include <gcodes.h>
#include <print_events.h>
#include <print_status.h>
#include <planner.h>

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
            fixed f = 0, feed0 = 0, feed1 = 0;
	    int32_t acc = def.acc_default;
            fixed x[3] = {0, 0, 0};
            for (i = 1; i < ncmds; i++) {
                switch (cmds[i].type) {
                case 'X':
                    x[0] = FIXED_ENCODE(cmds[i].val_f)/100;
                    break;
                case 'Y':
                    x[1] = FIXED_ENCODE(cmds[i].val_f)/100;
                    break;
                case 'Z':
                    x[2] = FIXED_ENCODE(cmds[i].val_f)/100;
                    break;
                case 'F':
                    f = FIXED_ENCODE(cmds[i].val_i);
                    break;
                case 'P':
                    feed0 = FIXED_ENCODE(cmds[i].val_i);
                    break;
                case 'L':
                    feed1 = FIXED_ENCODE(cmds[i].val_i);
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
        }
        case 28: {
            int i;
            int rx = 0, ry = 0, rz = 0;
            for (i = 1; i < ncmds; i++) {
                switch (cmds[i].type) {
                case 'X':
                    rx = 1;
                    break;
                case 'Y':
                    ry = 1;
                    break;
                case 'Z':
                    rz = 1;
                    break;
                }
            }
            if (!rx && !ry && !rz) {
                rx = 1;
                ry = 1;
                rz = 1;
            }

            planner_find_begin(rx, ry, rz, nid);
            return -E_OK;
        }
        case 30:
            planner_z_probe(nid);
            return -E_OK;
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
        return rc;
    }

    return handle_g_command(&frame);
}
