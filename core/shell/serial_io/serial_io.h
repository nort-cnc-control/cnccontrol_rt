#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <shell_base.h>
#include <serial_base.h>

void serial_io_init(void);

extern struct shell_cbs_s serial_io_shell_cbs;
extern struct serial_cbs_s serial_io_serial_cbs;
