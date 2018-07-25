#include <stddef.h>
#include <stdio.h>

#include "moves.h"
#include "control.h"

#include <gcode/gcodes.h>
#include <err.h>

#include <shell/shell.h>

#define MAX_CMDS 20

#define ANSWER_OK(str)  shell_print_answer(0, (str))
#define ANSWER_ERR(err, str)  shell_print_answer(err, (str))

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
			int32_t x[3] = {0, 0, 0};
			// x contains the relative coordinates
			for (i = 0; i < 3; i++)
				x[i] = -position.pos[i] * position.abs_crd;
			
			for (i = 1; i < ncmds; i++) {
				switch (cmds[i].type) {
				case 'X':
					x[0] += cmds[i].val_f;
					break;
				case 'Y':
					x[1] += cmds[i].val_f;
					break;
				case 'Z':
					x[2] += cmds[i].val_f;
					break;
				case 'F':
					set_speed(cmds[i].val_i);
					break;
				}
			}
			move_line_to(x);
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
				rz = 0;
			}
			find_begin(rx, ry, rz);
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

