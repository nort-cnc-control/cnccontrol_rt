#pragma once

#include <rdpos.h>
#include <stdint.h>
#include <stdbool.h>
#include <shell_base.h>
#include <serial_base.h>

void rdpos_io_init(void);

// RDPoS specific functions
void rdpos_io_retry(void);
void rdpos_io_close(void);
void rdpos_io_register_timeout_handlers(void (*retry)(bool), void (*close)(bool));

extern struct serial_cbs_s rdpos_io_serial_cbs;
extern struct shell_cbs_s rdpos_io_shell_cbs;
