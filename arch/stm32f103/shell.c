#include <stdbool.h>
#include <string.h>

#include <output/output.h>

#define MNUM 8
#define MLEN 120
static char messages[MNUM][MLEN];
static int mpos = 0;
static int mfirst = 0;
static int mnum = 0;

void uart_send(const uint8_t *data, ssize_t len);
void uart_pause(void);
void uart_continue(void);

const char *shell_get_message(size_t *len)
{
    if (mnum != 0)
    {
        *len = ((int)messages[mfirst][1]) << 8 | messages[mfirst][2];
        return messages[mfirst];
    }
    *len = 0;
    return NULL;
}

void shell_pop_message(void)
{
    if (mnum == 0)
        return;
    mnum--;
    mfirst = (mfirst + 1) % MNUM;
}

bool shell_add_message(const char *msg)
{
    if (mnum == MNUM)
        return false;
    memset(messages[mpos], 0, MLEN);
    strncpy(messages[mpos], msg, MLEN-1);
    mpos = (mpos + 1) % MNUM;
    mnum++;
    return true;
}

int shell_empty_slots(void)
{
    return MNUM - mnum;
}

bool shell_has_pending_messages(void)
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
        if (!shell_add_message(data))
            return -1;
    }
    else
    {
        uart_send(data, len);
    }
    return 0;
}

void shell_setup(void)
{
    output_set_write_fun(write_fun);
    output_control_set_fd(0);
    output_shell_set_fd(1);
}

