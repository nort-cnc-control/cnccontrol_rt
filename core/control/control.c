#include <stddef.h>
#include <stdio.h>

#include "moves.h"
#include "control.h"
#include "planner.h"

#include <gcode/gcodes.h>
#include <err.h>

#include <shell/shell.h>
#include <shell/print.h>

#define MAX_CMDS 20

#define ANSWER_OK(str)  shell_print_answer(0, (str))
#define ANSWER_ERR(err, str)  shell_print_answer(err, (str))

static void print_endstops(void)
{
	cnc_endstops stops = moves_get_endstops();
	shell_send_string("ok X: ");
	shell_send_char(stops.stop_x + '0');
	shell_send_string(" Y: ");
	shell_send_char(stops.stop_y + '0');
	shell_send_string(" Z: ");
	shell_send_char(stops.stop_z + '0');
	shell_send_string("\r\n");
}

static void print_position(void)
{	
	shell_send_string("ok X: ");
	shell_print_fixed_2(position.pos[0]);
	shell_send_string(" Y: ");
	shell_print_fixed_2(position.pos[1]);
	shell_send_string(" Z: ");
	shell_print_fixed_2(position.pos[2]);
	shell_send_string("\r\n");
}

static void send_ok(void)
{
	shell_send_string("ok\n\r");
}

int execute_g_command(const char *command)
{
	cmd_t allcmds[MAX_CMDS];
	const cmd_t *cmds = allcmds;
	int ncmds = 0;
	int i = 0, rc;

	if ((rc = parse_cmdline(command, MAX_CMDS, allcmds, &ncmds)) < 0) {
		ANSWER_ERR(-1, "parse error");
		return rc;
	}

	// skip line number(s)
	while (ncmds > 0 && cmds[0].type == 'N') {
		ncmds--;
		cmds++;
	}
	if (ncmds == 0)
		return -E_OK;

	// parse command line
	switch (cmds[0].type) {
	case 'G':
		switch (cmds[0].val_i) {
		case 0:
		case 1: {
			int32_t f = -1;
			int32_t x[3] = {0, 0, 0};
			int32_t init[3] = {0, 0, 0};
			// x contains the relative coordinates
			if (position.abs_crd)
			{
				for (i = 0; i < 3; i++)
					init[i] = position.pos[i];
			}
			for (i = 1; i < ncmds; i++) {
				switch (cmds[i].type) {
				case 'X':
					x[0] = cmds[i].val_f - init[0];
					break;
				case 'Y':
					x[1] = cmds[i].val_f - init[1];
					break;
				case 'Z':
					x[2] = cmds[i].val_f - init[2];
					break;
				case 'F':
					f = cmds[i].val_i;
					break;
				}
			}
			planner_line_to(x, f);
			planner_function(send_ok);
			break;
		}
		case 20:
			shell_print_answer(0, NULL);
			break;
		case 21:
			shell_print_answer(0, NULL);
			break;
		case 28: {
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
			planner_find_begin(rx, ry, rz);
			planner_function(send_ok);
			break;
		}
		case 90:
			position.abs_crd = 1;
			shell_print_answer(0, NULL);
			break;
		case 91:
			position.abs_crd = 0;
			shell_print_answer(0, NULL);
			break;
		default:
 			shell_print_answer(-1, "unknown command number");
			return -E_INCORRECT;
		}
		break;
	case 'M':
		switch (cmds[0].val_i) {
		case 99:
			// END
			return -E_OK;
		case 114:
			print_position();
			break;
		case 119:
			print_endstops();
			break;
		case 204:
			if (cmds[1].type != 'T') {
				shell_print_answer(-1, "expected: M204 Txx");
			}
			set_acceleration(cmds[1].val_i);
			break;
		case 999:
//			system_reset();
			break;
		default:
 			shell_print_answer(-1, "unknown command number");
			return -E_INCORRECT;
		}
		break;
	default:
 		shell_print_answer(-1, "unknown command");
		return -E_INCORRECT;
	}
	return E_OK;
}

