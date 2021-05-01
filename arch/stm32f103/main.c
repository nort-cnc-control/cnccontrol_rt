#include "config.h"

#ifdef CONFIG_LIBCORE
#include <control/moves/moves.h>
#include <control/planner/planner.h>
#include <control/control.h>
#include <output/output.h>
#include "steppers.h"
#endif

#include <stdio.h>
#include <shell.h>

#include "platform.h"
#include <net.h>
#include <iface.h>


#ifdef CONFIG_LIBCORE
static void init_steppers(void)
{
    gpio_definition gd;

    steppers_definition sd = {};
    steppers_config(&sd, &gd);
    init_control(&sd, &gd);
}
#endif

static struct
{
	uint32_t ip;
	uint16_t port;
} remote;

void udp_packet_handler(uint32_t remote_ip, uint16_t dport, uint16_t sport, const uint8_t *payload, size_t len)
{
	if (dport == CONFIG_UDP_PORT)
	{
		remote.ip = remote_ip;
		remote.port = sport;
		shell_data_received(payload, len);
		shell_data_completed();
	}
}

void shellSend(void)
{
	ssize_t len;
	const uint8_t *data = shell_pick_message(&len);
	if (data == NULL)
		return;
	libip_send_udp_packet(data, len, remote.ip, CONFIG_UDP_PORT, remote.port);
	shell_send_completed();
}

static void net_setup(void)
{
	const char *ipstr = CONFIG_IP_IPADDR;
	const char *macstr = CONFIG_ETHERNET_MAC_ADDR;

	unsigned mac[6];
	unsigned ip[4];
	sscanf(ipstr, "%u.%u.%u.%u", &ip[0], &ip[1], &ip[2], &ip[3]);
	sscanf(macstr, "%x:%x:%x:%x:%x:%x", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);

	uint32_t ipaddr = (ip[0] << 24) | (ip[1] << 16) | (ip[2] << 8) | (ip[3]);
	uint8_t macaddr[6] = {(uint8_t)(mac[0]), (uint8_t)(mac[1]), (uint8_t)(mac[2]), (uint8_t)(mac[3]), (uint8_t)(mac[4]), (uint8_t)(mac[5])};
	ifaceInitialise(macaddr);
	libip_init(ipaddr, macaddr);
	shell_setup(NULL, NULL);
}

/* main */
int main(void)
{
    hardware_setup();

#ifdef CONFIG_LIBCORE
    init_steppers();
    planner_lock();
    moves_reset();
#endif
	net_setup();
	
    shell_add_message("Hello", -1);

    while (true)
    {
#ifdef CONFIG_LIBCORE
        planner_pre_calculate();
#endif
		ifacePoll();
		shellSend();
    }

    return 0;
}
