#include <serial_sender.h>

#define BUFLEN 512

static char outbuf[BUFLEN];
static volatile int outlen;

volatile static struct
{
    uint8_t rdy : 1;
} serial_flags;

static serial_sender_cbs cbs;

volatile static int pos;

void serial_sender_init(serial_sender_cbs callbacks)
{
    cbs = callbacks;
    serial_flags.rdy = 1;
    pos = 0;
    outlen = 0;
}

void serial_sender_send_char(char c)
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

void serial_sender_char_transmitted(void)
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

bool serial_sender_completed(void)
{
    return (outlen == 0);
}
