#pragma once

#include <stdint.h>
#include <stdbool.h>

#include <enc28j60.h>

void net_setup(	struct enc28j60_state_s *eth_state,
		void (*on_send_completed_cb)(void),
		void (*on_packet_received_cb)(const char *data, size_t len));

int net_send(const uint8_t *data, ssize_t len);
void net_receive(void);
bool net_ready(void); 
