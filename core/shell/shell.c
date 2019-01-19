#include <stdint.h>
#include <string.h>
#include "shell.h"

#define BUFLEN 512

static char inbuf[BUFLEN];
static volatile uint8_t inlen;

static shell_cbs cbs;
volatile uint8_t echo;

void shell_init(shell_cbs cb)
{
    cbs = cb;
    inlen = 0;
    echo = 0;
}

void shell_echo_enable(int enable_echo)
{
    echo = enable_echo;
}


void shell_char_received(char c)
{
    switch (c)
    {
    case '\n':
    case '\r':
        inbuf[inlen] = 0;
        inlen = 0;
        if (echo)
            shell_send_string("\r\n");
        if (cbs.line_received)
            cbs.line_received(inbuf);
        break;
    case '\b':
    case 0x7F:
        if (inlen > 0)
        {
            inlen--;
            if (echo)
                shell_send_char('\b');
        }
        break;
    default:
        if ((inlen < BUFLEN - 1)) {
            inbuf[inlen++] = c;
            if (echo)
                shell_send_char(c);
        } else {
            if (echo)
                shell_send_char('^');
        }
        break;
    }
}

void shell_print_answer(int res, const char *ans)
{
    if (res == 0)
        shell_send_string("ok ");
    else
        shell_send_string("ERROR:");
    if (ans)
        shell_send_string(ans);
    shell_send_string("\r\n");
}

void shell_send_char(char c)
{
    cbs.send_char(c);
}

void shell_send_string(const char *str)
{
    int i;

    for (i = 0; str[i] != 0; i++)
        shell_send_char(str[i]);
}

