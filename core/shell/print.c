#include "shell.h"
#include "print.h"

void shell_print_dec(int32_t x)
{
	char buf[20];
	int i;
	if (x < 0)
	{
		x = -x;
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

void shell_print_fixed_2(int32_t x)
{
	if (x < 0)
	{
		shell_send_char('-');
		x = -x;
	}
	shell_print_dec(x / 100);
	shell_send_char('.');
	shell_print_dec((x % 100)/10);
	shell_print_dec(x % 10);
}

void shell_print_hex(uint32_t x)
{
	char buf[9];
	int i;
	if (x == 0)
	{
		shell_send_string("0x0");
		return;
	}
	for (i = 0; i < 20 && x > 0; i++)
	{
		buf[i] = x & 0xF;
		x >>= 4;
	}
	i--;
	shell_send_string("0x");
	while (i >= 0)
	{
		if (buf[i] <= 9)
			shell_send_char('0' + buf[i]);
		else
			shell_send_char('A' + buf[i] - 0xA);
		i--;
	}
}

