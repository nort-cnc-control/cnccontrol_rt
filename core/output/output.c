#include <output.h>
#include <string.h>

static int control_fd;
static int shell_fd;
static ssize_t (*write_fun)(int, const void *, ssize_t);

void output_control_set_fd(int fd)
{
    control_fd = fd;
}

void output_shell_set_fd(int fd)
{
    shell_fd = fd;
}

void output_set_write_fun(ssize_t (*write_f)(int, const void *, ssize_t))
{
    write_fun = write_f;
}

void output_control_write(const char *buf, ssize_t len)
{
    if (len < 0)
        len = strlen(buf);
    write_fun(control_fd, buf, len);
}

void output_shell_write(const char *buf, ssize_t len)
{
    if (len < 0)
        len = strlen(buf);
    write_fun(shell_fd, buf, len);
}
