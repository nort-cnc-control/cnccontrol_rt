#pragma once

#include <stdint.h>
#include <unistd.h>
#include <shell_base.h>

#define SHELL_BUFLEN 128

void shell_print_init(shell_cbs *callbacks);

void shell_send_string(const char *str);

void shell_print_answer(int res, const char *ans);
