#pragma once

#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

void uart_setup(void);

#ifdef CONFIG_LIBMODBUS
void uart_send_modbus(const uint8_t *buf, size_t len);
#endif

#ifdef CONFIG_BOARD_MEGA2560_CONTROL_UART
void uart_send_control(const uint8_t *buf, size_t len);
extern bool uart_message_received;
#endif
