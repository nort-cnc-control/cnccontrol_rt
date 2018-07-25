#pragma once

typedef struct {
    void (*line_received)(const char *line);
    void (*transmit_char)(char c);
} shell_cbs;

void shell_init(shell_cbs callbacks);

void shell_char_received(char c);

void shell_char_transmitted(void);

void shell_send_char(char c);
void shell_send_string(const char *str);

void shell_print_answer(int res, const char *ans);

