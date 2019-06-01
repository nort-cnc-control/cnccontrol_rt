#include <stddef.h>
#include <gcode_handler.h>
#include <shell_read.h>

static void command_received(const unsigned char *cmd, size_t len)
{
    execute_g_command(cmd, len);
}

void shell_read_init(shell_cbs *cbs)
{
    cbs->register_received_cb(command_received);
}
