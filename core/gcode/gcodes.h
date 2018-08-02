#pragma once

#include <stdint.h>

#define MAX_CMDS 6

typedef struct {
        char type;
        union {
                int32_t val_i;
                int32_t val_f;
        };
} gcode_cmd_t;

typedef struct {
	int num;
	gcode_cmd_t cmds[MAX_CMDS];
} gcode_frame_t;

int parse_cmdline(const char *str, gcode_frame_t *frame);

