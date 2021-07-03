#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include <shell.h>

#ifdef CONFIG_LIBCORE
#include <output/output.h>
#include <control/commands/gcode_handler/gcode_handler.h>
#endif

#ifdef CONFIG_LIBMODBUS
#include <modbus.h>
#endif

uint32_t shell_fails = 0;

static char messages[SHELL_RING_LEN][SHELL_MSG_LEN];
static int mpos = 0;
static int mfirst = 0;
static int mnum = 0;

static char input_buffer[SHELL_MSG_LEN];

#ifdef CONFIG_COPY_COMMAND
static char command_buffer[SHELL_MSG_LEN-3];
#endif
static int input_pos = 0;

static void (*debug_send)(const uint8_t *data, ssize_t len);
static void (*uart_send)(const uint8_t *data, size_t len);

static void shell_pop_message(void)
{
    if (mnum == 0)
        return;
    mnum--;
    mfirst = (mfirst + 1) % SHELL_RING_LEN;
}

bool shell_add_message(const char *msg, ssize_t len)
{
    if (mnum == SHELL_RING_LEN)
    {
        shell_fails++;
        return false;
    }
    if (len < 0)
        len = strlen(msg);

    len = len < SHELL_MSG_LEN - 2 ? len : SHELL_MSG_LEN - 2;
    memset(messages[mpos], 0, SHELL_MSG_LEN);
    messages[mpos][0] = len >> 8;
    messages[mpos][1] = len;
    strncpy(messages[mpos]+2, msg, len);
    mpos = (mpos + 1) % SHELL_RING_LEN;
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
    if (mnum > 0)
    {
        *len = ((int)messages[mfirst][0]) << 8 | messages[mfirst][1];
        return messages[mfirst] + 2;
    }
    *len = -1;
    return NULL;
}

int shell_empty_slots(void)
{
    return SHELL_RING_LEN - mnum;
}

static int hex2dig(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 0xA;
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 0xA;
    return -1;
}

bool shell_data_received(const char *data, ssize_t len)
{
    int i;
    if (len < 0)
        len = strlen(data);

    if (input_pos + len > SHELL_MSG_LEN)
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
#ifdef CONFIG_COPY_COMMAND
        memcpy(command_buffer, input_buffer+3, input_pos-3);
        execute_g_command(command_buffer, input_pos - 3);
#else
        execute_g_command(input_buffer+3, input_pos - 3);
#endif
    }
#endif

#ifdef CONFIG_LIBMODBUS
    else if (input_pos >= 3 && !memcmp(input_buffer, "MB:", 3))
    {
        int i;
        const char *buf = input_buffer + 3;
        uint8_t addrs[4], vals[4], devids[4];
        memcpy(devids, buf, 4);
        memcpy(addrs, buf+4+1, 4);
        memcpy(vals, buf+4+1+4+1, 4);
        for (i = 0; i < 4; i++)
        {
            devids[i] = hex2dig(devids[i]);
            addrs[i] = hex2dig(addrs[i]);
            vals[i] = hex2dig(vals[i]);
        }
        uint16_t addr  =  addrs[0] * 0x1000 +  addrs[1] * 0x100 +  addrs[2] * 0x10 +  addrs[3];
        uint16_t val   =   vals[0] * 0x1000 +   vals[1] * 0x100 +   vals[2] * 0x10 +   vals[3];
        uint16_t devid = devids[0] * 0x1000 + devids[1] * 0x100 + devids[2] * 0x10 + devids[3];

#define PREAMBLE 0
        uint8_t buffer[40];
        memset(buffer, 0, PREAMBLE);
        ssize_t wrlen = modbus_fill_write_ao(buffer + PREAMBLE + MODBUS_HEADER_LEN, addr, val);
        ssize_t mblen = modbus_fill_header(buffer + PREAMBLE,  devid, FUNCTION_WRITE_AO, wrlen);
        memset(buffer + PREAMBLE + mblen, 0, PREAMBLE);

        /* send to UART */
        if (uart_send)
            uart_send(buffer, mblen + 2 * PREAMBLE);
#undef PREAMBLE
    }
#endif
    else if (input_pos >= 5 && !memcmp(input_buffer, "EXIT:", 5))
    {
        // Do nothing
    }
#ifdef CONFIG_ECHO
    else if (input_pos >= 5 && !memcmp(input_buffer, "ECHO:", 5))
    {
        // Send echo back
        shell_add_message(input_buffer, input_pos);
    }
#endif
    else
    {
        char buf[SHELL_MSG_LEN-2];
        int l = snprintf(buf, 100, "Unknown command: %i [%.*s]", input_pos, input_pos, input_buffer);
        shell_add_message(buf, l);
    }

    input_pos = 0;
}

void shell_setup(void (*debug_send_fun)(const uint8_t *, ssize_t), void (*uart_send_fun)(const uint8_t *data, size_t len))
{
    uart_send = uart_send_fun;
    debug_send = debug_send_fun;
#ifdef CONFIG_LIBCORE
    output_set_write_fun(write_fun);
    output_control_set_fd(0);
    output_shell_set_fd(1);
#endif
}

