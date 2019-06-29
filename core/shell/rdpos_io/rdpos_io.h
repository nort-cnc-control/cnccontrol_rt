#pragma once

#include <rdp.h>
#include <stdint.h>
#include <stdbool.h>
#include <shell_base.h>
#include <serial_base.h>

void rdpos_io_init(void);

// RDPoS specific functions
void rdpos_io_clock(int dt);

extern struct serial_cbs_s rdpos_io_serial_cbs;
extern struct shell_cbs_s rdpos_io_shell_cbs;
