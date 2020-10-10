#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>

#include "arch-defs.h"

struct modbus_header_s
{
    uint8_t address;
    uint8_t function;
};

struct modbus_write_ao_s
{
    uint16_t reg_h : 8;
    uint16_t reg_l : 8;

    uint16_t val_h : 8;
    uint16_t val_l : 8;
};

#define MODBUS_HEADER_LEN sizeof(struct modbus_header_s)
#define MODBUS_WRITE_AO_LEN sizeof(struct modbus_write_ao_s)

#define FUNCTION_READ_AO  0x03
#define FUNCTION_WRITE_AO 0x06

ssize_t modbus_fill_write_ao(uint8_t *buf, uint16_t reg, uint16_t val);
ssize_t modbus_fill_header(uint8_t *buf, uint8_t address, uint8_t function, size_t datalen);

