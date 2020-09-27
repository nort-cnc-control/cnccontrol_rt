#pragma once

#include <stdint.h>
#include <stdbool.h>

#include <enc28j60.h>

void net_setup(	struct enc28j60_state_s *eth_state,
		const uint8_t *(*pick_message_cb)(ssize_t *),
                void (*on_send_completed)(void),
		void (*on_packet_received_cb)(const char *data, size_t len));

void net_send(void);
void net_receive(void);
bool net_ready(void); 
