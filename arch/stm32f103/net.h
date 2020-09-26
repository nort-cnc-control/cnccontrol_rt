#pragma once

#include <stdint.h>
#include <stdbool.h>

void net_setup(void (*on_send_completed_cb)(void), void (*on_packet_received_cb)(const char *data, size_t len));
int net_send(const uint8_t *data, ssize_t len);
void net_receive(void);
bool net_ready(void); 
