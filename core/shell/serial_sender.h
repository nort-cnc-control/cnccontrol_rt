#pragma once

#include <stdint.h>

typedef struct {
    void (*transmit_char)(char c);
} serial_sender_cbs;

void serial_sender_send_char(char c);

void serial_sender_char_transmitted(void);

