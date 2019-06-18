#pragma once

#include <stddef.h>
#include <stdint.h>

struct serial_cbs_s {
    void (*register_byte_transmit)(void (*f)(uint8_t));
    void (*byte_transmitted)(void);
    void (*byte_received)(unsigned char);
};
