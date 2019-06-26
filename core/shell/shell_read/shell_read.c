#include <stddef.h>
#include <gcode_handler.h>
#include <shell_read.h>
#include <shell_print.h>
#include <stdio.h>

static void command_received(const unsigned char *cmd, size_t len)
{
    printf("Received: %.*s\n", len, cmd);
    execute_g_command(cmd, len);
}

void shell_read_init(struct shell_cbs_s *cbs)
{
    cbs->register_received_cb(command_received);
}
