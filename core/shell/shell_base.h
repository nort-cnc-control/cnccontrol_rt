#pragma once

#include <stddef.h>
#include <stdbool.h>

struct shell_cbs_s {
    void (*register_received_cb)(void (*)(const unsigned char *, size_t));
    void (*send_buffer)(const unsigned char *buf, size_t len);
    void (*register_sended_cb)(void (*)(void));
    void (*register_connected_cb)(void (*)(void));
    void (*register_disconnected_cb)(void (*)(void));
};
