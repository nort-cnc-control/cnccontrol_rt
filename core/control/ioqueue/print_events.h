
#pragma once

#include <stdint.h>

void send_queued(int nid);
void send_started(int nid);
void send_completed(int nid);
void send_completed_with_pos(int nid, const int32_t *pos);
void send_dropped(int nid);
void send_failed(int nid);

void send_ok(int nid);
void send_error(int nid, const char *err);
void send_warning(int nid, const char *err);
