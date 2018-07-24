#pragma once

typedef struct {
    void (*line_received)(const char *line);
    void (*transmit_char)(char c);
} serial_cbs;

void serial_init(serial_cbs callbacks);

void serial_char_received(char c);

void serial_char_transmitted(void);

void serial_send_string(char *str, int len);

void print_answer(int res, char *ans, int anslen);
