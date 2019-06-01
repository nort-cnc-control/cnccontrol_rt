#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <shell_base.h>


typedef struct serial_io_cbs_s
{
    void (*transmit_char)(unsigned char c);    
} serial_io_cbs;

void serial_io_char_received(unsigned char c);
void serial_io_char_transmitted(void);
void serial_io_init(serial_io_cbs *callbacks);

extern shell_cbs serial_io_shell_cbs;
