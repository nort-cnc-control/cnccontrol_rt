
#include "print.h"
#include "shell.h"
#include <planner.h>

void send_queued(int nid)
{
	int q = empty_slots();
	shell_send_string("queued");
	shell_send_string(" N:");
	shell_print_dec(nid);
	shell_send_string(" Q:");
	shell_print_dec(q);
	shell_send_string("\r\n");
}

void send_started(int nid)
{
	int q = empty_slots();
	shell_send_string("started");
	shell_send_string(" N:");
	shell_print_dec(nid);
	shell_send_string(" Q:");
	shell_print_dec(q);
	shell_send_string("\r\n");
}

void send_completed(int nid)
{
	int q = empty_slots();
	shell_send_string("completed");
	shell_send_string(" N:");
	shell_print_dec(nid);
	shell_send_string(" Q:");
	shell_print_dec(q);
	shell_send_string("\r\n");
}

void send_ok(int nid)
{
	send_queued(nid);
	send_started(nid);
	send_completed(nid);
}

void send_error(int nid, const char *err)
{
	int q = empty_slots();
	shell_send_string("error");
	shell_send_string(" N:");
	shell_print_dec(nid);
	shell_send_string(" ");
	shell_send_string(err);
	shell_send_string("\r\n");
}
