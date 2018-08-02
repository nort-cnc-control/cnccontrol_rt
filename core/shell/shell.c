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
    echo = 0;
}

void shell_echo_enable(int enable_echo)
{
	echo = enable_echo;
}

void shell_send_char(char c)
{
	if (outlen == BUFLEN)
		return;
	outbuf[outlen++] = c;
	if (outlen == 1 && serial_flags.rdy == 1)
	{
		pos = 1;
		serial_flags.rdy = 0;
		cbs.transmit_char(outbuf[0]);
	}
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

void shell_char_transmitted(void)
{
	if (outbuf[pos] && pos < outlen) {
		int p = pos;
		pos++;
        	cbs.transmit_char(outbuf[p]);
    	}
    	else {
        	outlen = 0;
        	pos = 0;
        	serial_flags.rdy = 1;
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

void shell_send_string(const char *str)
{
    int i;

    for (i = 0; str[i] != 0; i++)
        shell_send_char(str[i]);
}

