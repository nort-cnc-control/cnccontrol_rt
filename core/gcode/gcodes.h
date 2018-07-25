#pragma once

#include <stdint.h>

typedef struct {
        char type;
        union {
                int32_t val_i;
                int32_t val_f;
        };
} cmd_t;

int parse_cmdline(const char *str, int maxcmds, cmd_t *cmds, int *ncmd);

