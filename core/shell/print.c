#include "shell.h"
#include "print.h"

void shell_print_dec(int32_t x)
{
	char buf[20];
	int i;
	if (x < 0)
	{
		x *= -1;
		shell_send_char('-');
	}
	if (x == 0)
	{
		shell_send_char('0');
		return;
	}
	for (i = 0; i < 20 && x > 0; i++)
	{
		buf[i] = x % 10;
		x /= 10;
	}
	i--;
	while (i >= 0)
	{
		shell_send_char('0' + buf[i]);
		i--;
	}
}

