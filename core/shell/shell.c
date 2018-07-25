#include <stdint.h>
#include <string.h>
#include "shell.h"

#define BUFLEN 256

static char inbuf[BUFLEN];
static volatile uint8_t inlen;

static char outbuf[BUFLEN];
static volatile int outlen;

volatile static int pos;
volatile static struct
{
    uint8_t rdy : 1;
    uint8_t str : 1;
} serial_flags;

volatile uint8_t echo;

static shell_cbs cbs;

void shell_init(shell_cbs callbacks)
{
    cbs = callbacks;
    serial_flags.rdy = 1;
    pos = 0;
    outlen = 0;
    inlen = 0;
    echo = 1;
}

static void send_char(char c)
{
    serial_flags.str = 0;
    if (cbs.transmit_char)
        cbs.transmit_char(c);
}

void shell_char_received(char c)
{
    if (c != '\n' && c != '\r')
    {
        if (c == '\b' || c == 0x7F)
        {
            if (inlen > 0)
                inlen--;
            if (echo)
                send_char('\b');
        }
        else
        {
            if (echo)
            {
                if (inlen >= BUFLEN)
                    send_char('^');
                else
                    send_char(c);
            }
            if ((inlen < BUFLEN - 1))
                inbuf[inlen++] = c;
        }
    }
    else
    {
        inbuf[inlen] = 0;
        inlen = 0;
        if (echo)
            shell_send_string("\r\n", 2);
        if (cbs.line_received)
            cbs.line_received(inbuf);
    }
}

void shell_char_transmitted(void)
{
    if (outbuf[pos] && pos < outlen)
    {
        send_char(outbuf[pos]);
        pos++;
    }
    else
    {
        outlen = 0;
        pos = 0;
        serial_flags.rdy = 1;
    }
}

void shell_print_answer(int res, char *ans, int anslen)
{
    if (res == 0)
        shell_send_string("ok ", 3);
    else
        shell_send_string("ok ERROR:", 9);
    if (ans)
        shell_send_string(ans, anslen);
    shell_send_string("\r\n", 2);
}

void shell_send_string(char *str, int len)
{
    int i;

    if (len == 0)
        return;
    if (len < 0) {
        const char *p = str;
        len = 0;
        while (*p != 0)
            p++;
        len = p - str;
    }
    for (i = 0; str[i] != 0 && i < len && outlen < BUFLEN; i++)
        outbuf[outlen++] = str[i];
    if (serial_flags.rdy == 1)
    {
        serial_flags.rdy = 0;
        pos = 1;
        send_char(outbuf[0]);
    }
}
