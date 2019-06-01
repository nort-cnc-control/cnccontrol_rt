#include <stdio.h>
#include <test_io.h>

static void (*cb_line_received)(const unsigned char *, size_t);
static void (*cb_sended)(void);

static void send_buffer(const unsigned char *buf, size_t len)
{
    printf("%s\n\r", buf);
    cb_sended();
}

static void register_received_cb(void (*f)(const unsigned char *, size_t))
{
    cb_line_received = f;
}

static void register_sended_cb(void (*f)(void))
{
    cb_sended = f;
}

shell_cbs test_io_shell_cbs = {
    .register_received_cb = register_received_cb,
    .register_sended_cb = register_sended_cb,
    .send_buffer = send_buffer,
};
