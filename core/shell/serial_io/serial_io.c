#include <serial_io.h>
#include <string.h>
#include <stdbool.h>

static struct
{
    uint8_t rdy : 1;
} opts;

#define BUFLEN 128

static unsigned char inbuf[BUFLEN];
static int inlen;

static unsigned char outbuf[BUFLEN + 2];
static int outlast;
static int outpos;

static void (*cb_line_received)(const unsigned char *, size_t);
static void (*cb_sended)(void);
static void (*cb_byte_transmit)(uint8_t b);


static void add_char(char c)
{
    if ((inlen < BUFLEN - 1))
    {
        inbuf[inlen++] = c;
    }
}

void serial_io_init(void)
{
    opts.rdy = 1;
    outpos = 0;
    outlast = 0;
}

static void serial_io_char_received(unsigned char c)
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
        opts.rdy = 0;
        int p = outpos;
        outpos += 1;
        outpos %= sizeof(outbuf);
        if (cb_byte_transmit)
            cb_byte_transmit(outbuf[p]);
    }
}

static void serial_io_char_transmitted(void)
{
    if (outpos != outlast)
    {
        int p = outpos;
        outpos += 1;
        outpos %= sizeof(outbuf);
        if (cb_byte_transmit)
            cb_byte_transmit(outbuf[p]);
    }
    else
    {
        opts.rdy = 1;
        cb_sended();
    }
}

static void send_buffer(const unsigned char *buf, size_t len)
{
    int i;
    for (i = 0; i < len; i++)
    {
        outbuf[outlast] = buf[i];
        outlast += 1;
        outlast %= sizeof(outbuf);
    }
    outbuf[outlast] = '\n';
    outlast += 1;
    outlast %= sizeof(outbuf);
    outbuf[outlast] = '\r';
    outlast += 1;
    outlast %= sizeof(outbuf);

    if (opts.rdy == 1)
        serial_io_start_transmit();
}

static void register_received_cb(void (*f)(const unsigned char *, size_t))
{
    cb_line_received = f;
}

static void register_sended_cb(void (*f)(void))
{
    cb_sended = f;
}

static void register_byte_transmit(void (*f)(unsigned char))
{
    cb_byte_transmit = f;
}

static void register_connected_cb(void (*f)(void))
{
    f();
}

static void register_disconnected_cb(void (*f)(void))
{
}

struct shell_cbs_s serial_io_shell_cbs = {
    .register_received_cb = register_received_cb,
    .register_sended_cb = register_sended_cb,
    .send_buffer = send_buffer,
    .register_connected_cb = register_connected_cb,
    .register_disconnected_cb = register_disconnected_cb,
};

struct serial_cbs_s serial_io_serial_cbs = {
    .register_byte_transmit = register_byte_transmit,
    .byte_received = serial_io_char_received,
    .byte_transmitted = serial_io_char_transmitted,
};
