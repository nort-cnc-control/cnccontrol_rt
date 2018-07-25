#pragma once

typedef struct {
    void (*line_received)(const char *line);
    void (*transmit_char)(char c);
} shell_cbs;

void shell_init(shell_cbs callbacks);

void shell_char_received(char c);

void shell_char_transmitted(void);

void shell_send_string(char *str, int len);

void shell_print_answer(int res, char *ans, int anslen);

