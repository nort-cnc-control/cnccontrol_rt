#include <stdbool.h>
#include <string.h>

#include "shell.h"

#include <output/output.h>
#include <control/commands/gcode_handler/gcode_handler.h>

#define MNUM 8
#define MLEN 120
static char messages[MNUM][MLEN];
static int mpos = 0;
static int mfirst = 0;
static int mnum = 0;

static char input_buffer[MLEN];
static int input_pos = 0;

static void (*debug_send)(const uint8_t *data, ssize_t len);

static void shell_pop_message(void)
{
    if (mnum == 0)
        return;
    mnum--;
    mfirst = (mfirst + 1) % MNUM;
}

bool shell_add_message(const char *msg, ssize_t len)
{
    if (mnum == MNUM)
        return false;
    if (len < 0)
        len = strlen(msg);

    len = len < MLEN - 2 ? len : MLEN - 2;
    memset(messages[mpos], 0, MLEN);
    messages[mpos][0] = len >> 8;
    messages[mpos][1] = len;
    strncpy(messages[mpos]+2, msg, len);
    mpos = (mpos + 1) % MNUM;
    mnum++;
    return true;
}

static bool shell_has_pending_messages(void)
{
    return mnum > 0;
}

static ssize_t write_fun(int fd, const void *data, ssize_t len)
{
    int i;
    if (len < 0)
        len = strlen((const char *)data);
    if (fd == 0)
    {
        if (!shell_add_message(data, len))
            return -1;
    }
    else
    {
        if (debug_send)
            debug_send(data, len);
    }
    return 0;
}

/* Public methods */

void shell_send_completed(void)
{
    shell_pop_message();
}

const uint8_t *shell_pick_message(ssize_t *len)
{
    if (mnum != 0)
    {
        *len = ((int)messages[mfirst][0]) << 8 | messages[mfirst][1];
        return messages[mfirst] + 2;
    }
    *len = -1;
    return NULL;
}

int shell_empty_slots(void)
{
    return MNUM - mnum;
}

bool shell_data_received(const char *data, ssize_t len)
{
    int i;
    if (len < 0)
        len = strlen(data);

    if (input_pos + len > MLEN)
    {
        input_pos = 0;
        return false;
    }
    memcpy(input_buffer + input_pos, data, len);
    for (i = 0; i < len; i++)
    {
        if (input_buffer[i + input_pos] == '\n' || input_buffer[i + input_pos] == '\r')
            input_buffer[i + input_pos] = ' ';
    }

    input_pos += len;
    return true;
}

void shell_data_completed(void)
{
    if (input_pos >= 6 && !memcmp(input_buffer, "START:", 6))
    {
        // Do nothing
    }
#ifdef CONFIG_LIBCORE
    else if (input_pos >= 3 && !memcmp(input_buffer, "RT:", 3))
    {
        execute_g_command(input_buffer + 3, input_pos - 3);
    }
#endif

#ifdef CONFIG_LIBMODBUS
    else if (input_pos >= 3 && !memcmp(input_buffer, "MB:", 3))
    {
        // TODO: add modbus code
    }
#endif
    else if (input_pos >= 5 && !memcmp(input_buffer, "EXIT:", 5))
    {
        // Do nothing
    }
    else
    {
        // Error
    }

    input_pos = 0;
}

void shell_setup(void (*debug_send_fun)(const uint8_t *, ssize_t))
{
    debug_send = debug_send_fun;
    output_set_write_fun(write_fun);
    output_control_set_fd(0);
    output_shell_set_fd(1);
}

