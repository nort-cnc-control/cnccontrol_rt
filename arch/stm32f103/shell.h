#pragma once

#include <stdint.h>
#include <unistd.h>

void shell_setup(void (*debug_send_fun)(const uint8_t *, ssize_t), int (*shell_send_fun)(const uint8_t *, ssize_t));

/* Output methods */
void shell_send_completed(void);
int shell_empty_slots(void);

/* Input methods */
bool shell_data_received(const char *data, ssize_t len);
void shell_data_completed(void);

