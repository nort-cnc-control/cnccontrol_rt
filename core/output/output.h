#pragma once
#include <unistd.h>

void output_control_set_fd(int fd);
void output_shell_set_fd(int fd);

void output_set_write_fun(ssize_t (*write_f)(int, const void *, ssize_t));

void output_control_write(const char *buf, ssize_t len);
void output_shell_write(const char *buf, ssize_t len);
