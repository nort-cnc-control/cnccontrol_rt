#pragma once

int execute_g_command(const char *command);

void send_queued(int nid);
void send_started(int nid);
void send_completed(int nid);
void send_ok(int nid);
void send_error(int nid, const char *err);

