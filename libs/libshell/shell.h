#pragma once

#include <stdint.h>
#include <unistd.h>
#include <stdbool.h>

#include "arch-defs.h"

void shell_setup(void (*debug_send_fun)(const uint8_t *, ssize_t), void (*uart_send_fun)(const uint8_t *data, size_t len));

/* Output methods */
void shell_send_completed(void);
const uint8_t *shell_pick_message(ssize_t *len);
int shell_empty_slots(void);
bool shell_add_message(const char *msg, ssize_t len);

/* Input methods */
bool shell_data_received(const char *data, ssize_t len);
void shell_data_completed(void);

