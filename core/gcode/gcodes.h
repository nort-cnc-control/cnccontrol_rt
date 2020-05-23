#pragma once

#include <stdint.h>
#include <stddef.h>
#include <err.h>

#define MAX_CMDS 16

typedef struct {
    char type;
    union {
        int32_t val_i;
        double val_f;
    };
} gcode_cmd_t;

typedef struct {
    int num;
    gcode_cmd_t cmds[MAX_CMDS];
} gcode_frame_t;

int parse_cmdline(const unsigned char *str, size_t len, gcode_frame_t *frame);
