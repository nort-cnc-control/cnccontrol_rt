#include <err.h>
#include <gcode/gcodes.h>
#include <stdio.h>

static int islast(char c)
{
	return c == 0 || c == ';' || c == '\n';
}

static int read_int(const char **str, int32_t *val)
{
	if ((**str >= '0' && **str <= '9') || **str == '-') {
		int32_t v = 0;
		int8_t minus = 1;
		if (**str == '-') {
			minus = -1;
			(*str)++;
		}
		while (**str >= '0' && **str <= '9') {
			v *= 10;
			v += (**str - '0');
			(*str)++;
		}
		v *= minus;
		*val = v;
		return E_OK;
	}
	return -E_BADNUM;
}

static uint8_t hex_decode(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return 0xFF;
}

static uint8_t is_hex(char c)
{
	if (c >= '0' && c <= '9')
		return 1;
	if (c >= 'a' && c <= 'f')
		return 1;
	if (c >= 'A' && c <= 'F')
		return 1;
	return 0;
}

static int read_hex(const char **str, int32_t *val)
{
	if (is_hex(**str)) {
		int32_t v = 0;
		while (is_hex(**str)) {
			v *= 16;
			v += hex_decode(**str);
			(*str)++;
		}
		*val = v;
		return E_OK;
	}
	return -E_BADNUM;
}

static int read_fixed(const char **str, int32_t *val)
{
	if ((**str >= '0' && **str <= '9') || **str == '-' || **str == '.') {
		int8_t minus = 1;
		int32_t v = 0;
		if (**str == '-') {
			minus = -1;
			(*str)++;
		}
		while (**str >= '0' && **str <= '9') {
			v *= 10;
			v += (**str - '0');
			(*str)++;
		}
		v *= 100;
		if (**str == '.') {
			uint8_t s = 0;
			uint8_t ns = 0;
			(*str)++;
			while (**str >= '0' && **str <= '9') {
				if (ns < 2) {
					s *= 10;
					s += (**str - '0');
				}
				(*str)++;
				ns++;
			}
			if (ns == 1)
				s*= 10;
			v += s;
		}
		v *= minus;
		*val = v;
		return E_OK;
	}
	return -E_BADNUM;
}

static int parse_element(const char **str, gcode_cmd_t *cmd)
{
	if (str == NULL || *str == NULL)
		return -E_NULL;

	while (**str == ' ')
		(*str)++;
	
	if (islast(**str)) {
		cmd->type = 0;
		return E_OK;
	}
	
	if (!((**str >= 'A' && **str <= 'Z') || **str == '*'))
		return -E_INCORRECT;

	switch ((*str)[0]) {
	case 'G':
	case 'M':
	case 'P':
	case 'L':
	case 'F':
	case 'N':
	case 'T':
	case 'S':
		cmd->type = **str;
		(*str)++;
		if (read_int(str, &(cmd->val_i)))
			return -E_BADNUM;
		break;
	case 'X':
	case 'Y':
	case 'Z':
	case 'E':
	case 'R':
	case 'D':
	case 'I':
	case 'J':
	case 'K':
		cmd->type = **str;
		(*str)++;
		if (**str == ' ' || **str == '\n' || **str == 0) {
			cmd->val_f = 0;
			break;
		}
		if (read_fixed(str, &(cmd->val_f)))
			return -E_BADNUM;
		break;
	case '*':
		cmd->type = '*';
		(*str)++;
		if (read_hex(str, &(cmd->val_i)))
			return -E_BADNUM;

		break;
	default:
		return -E_INCORRECT;
	}
//	while (**str != ' ' && **str != 0 && **str != '\n')
//		(*str)++;

	return E_OK;
}

int parse_cmdline(const char *str, gcode_frame_t *frame)
{
	int i = 0, rc;
	while (*str != 0 && i < MAX_CMDS)
       	{
		if ((rc = parse_element(&str, &(frame->cmds[i]))) < 0) {
			return rc;
		}
		if (frame->cmds[i].type == 0)
			break;
		i++;
	}
	frame->num  = i;
	return E_OK;
}

