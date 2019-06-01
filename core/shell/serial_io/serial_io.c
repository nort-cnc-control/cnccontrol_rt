#include <serial_io.h>


static struct
{
    uint8_t rdy : 1;
} opts;

#define BUFLEN 128

static unsigned char inbuf[BUFLEN];
static int inlen;

static const unsigned char *outbuf;
static int outlen;
static int outpos;

static void (*cb_line_received)(const unsigned char *, size_t);
static void (*cb_sended)(void);

static serial_io_cbs *cbs;

static void add_char(char c)
{
    if ((inlen < BUFLEN - 1))
    {
        inbuf[inlen++] = c;
    }
}

void serial_io_init(serial_io_cbs *callbacks)
{
    cbs = callbacks;
    opts.rdy = 1;
    outpos = 0;
}

void serial_io_char_received(unsigned char c)
{
    switch (c)
    {
    case '\n':
    case '\r':
        inbuf[inlen] = 0;
        if (cb_line_received)
            cb_line_received(inbuf, inlen);
        inlen = 0;
        break;
    default:
        add_char(c);
        break;
    }
}

static void serial_io_start_transmit(void)
{
    if (opts.rdy == 1)
    {
        outpos = 1;
        opts.rdy = 0;
        cbs->transmit_char(outbuf[0]);
    }
}

void serial_io_char_transmitted(void)
{
    if (outbuf[outpos] && outpos < outlen)
    {
        int p = outpos;
        outpos++;
        cbs->transmit_char(outbuf[p]);
    }
    else {
        outlen = 0;
        outpos = 0;
        opts.rdy = 1;
        cb_sended();
    }
}

static void send_buffer(const unsigned char *buf, size_t len)
{
    outbuf = buf;
    outlen = len;
    outpos = 0;
}

static void register_received_cb(void (*f)(const unsigned char *, size_t))
{
    cb_line_received = f;
}

static void register_sended_cb(void (*f)(void))
{
    cb_sended = f;
}

shell_cbs serial_io_shell_cbs = {
    .register_received_cb = register_received_cb,
    .register_sended_cb = register_sended_cb,
    .send_buffer = send_buffer,
};
