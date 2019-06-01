#pragma once

#include <stddef.h>

typedef struct shell_cbs_s {
    void (*register_received_cb)(void (*)(const unsigned char *, size_t));
    void (*send_buffer)(const unsigned char *buf, size_t len);
    void (*register_sended_cb)(void (*)(void));
} shell_cbs;
