#pragma once

#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

void uart_setup(int baudrate);

void uart_send(const uint8_t *buf, size_t len);

