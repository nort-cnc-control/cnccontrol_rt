#pragma once

#include <stdint.h>
#include <unistd.h>
#include <shell_base.h>
#include <stdbool.h>

#define SHELL_BUFLEN 128

void shell_print_init(struct shell_cbs_s *callbacks);

void shell_send_string(const char *str);

void shell_print_answer(int res, const char *ans);

bool shell_connected(void);
