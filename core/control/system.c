#include <system.h>

static void (*rb)(void);

void system_init(void (*reboot)(void))
{
    rb = reboot;
}

void system_reboot(void)
{
    rb();
}

