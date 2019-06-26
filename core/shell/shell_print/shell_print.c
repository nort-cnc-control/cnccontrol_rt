#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <err.h>
#include <shell_print.h>
#include <stdbool.h>

#define NUMBUF 12

static unsigned char outbuf[NUMBUF][SHELL_BUFLEN];
static volatile int outpos, outlast, sended;
static struct shell_cbs_s *cbs;

static void buffer_sended(void)
{
    if (outpos == -1)
        outpos = sended;
    
    outlast = sended;
    memset(outbuf[sended], 0, SHELL_BUFLEN);

    if ((sended + 1) % NUMBUF != outpos)
    {
        sended = (sended + 1) % NUMBUF;
        cbs->send_buffer(outbuf[sended], strlen(outbuf[sended]));
    }
    else
    {
        sended = -1;
    }
}

static bool connected = false;

void on_connect(void)
{
    outpos = 0;
    outlast = NUMBUF - 1;
    sended = -1;
    connected = true;
}

void on_disconnect(void)
{
    outpos = 0;
    outlast = NUMBUF - 1;
    sended = -1;
    connected = false;
}

void shell_print_init(struct shell_cbs_s *cb)
{
    cbs = cb;
    outpos = 0;
    outlast = NUMBUF - 1;
    sended = -1;
    cbs->register_sended_cb(buffer_sended);
    cbs->register_connected_cb(on_connect);
    cbs->register_disconnected_cb(on_disconnect);
}

void shell_send_string(const char *str)
{
    if (outpos != -1)
    {
        int first = (sended == -1);
        int pos = outpos;
        strncpy(outbuf[pos], str, SHELL_BUFLEN);

        if (outpos != outlast)
        {
            outpos = (outpos + 1) % NUMBUF;
        }
        else
        {
            outpos = -1;
            outlast = -1;
        }

        if (first)
        {
            sended = pos;
            cbs->send_buffer(outbuf[sended], strlen(outbuf[sended]));
        }
    }
    else
    {
        /* No free slots */
    }
}

void shell_send_result(int res, const char *ans)
{
    unsigned char buf[SHELL_BUFLEN];
    if (ans == NULL)
        ans = "";

    if (res == -E_OK)
        snprintf(buf, SHELL_BUFLEN, "ok %s", ans);
    else
        snprintf(buf, SHELL_BUFLEN, "ERROR (%i): %s", res, ans);
    buf[SHELL_BUFLEN-1] = 0;
    shell_send_string(buf);
}

bool shell_connected(void)
{
    return connected;
}
