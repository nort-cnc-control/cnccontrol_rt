#include "config.h"

#ifdef CONFIG_LIBCORE
#include <control/moves/moves.h>
#include <control/planner/planner.h>
#include <control/control.h>
#include <output/output.h>
#include "steppers.h"
#endif

#include <stdbool.h>
#include <unistd.h>

#include <shell.h>
#include <iface.h>
#include <net.h>

#include "platform.h"

#include <FreeRTOS.h>
#include <task.h>

#ifdef CONFIG_LIBCORE
static void init_steppers(void)
{
	gpio_definition gd;

	steppers_definition sd = {};
	steppers_config(&sd, &gd);
	init_control(&sd, &gd);
}

static void preCalculateTask(void *args)
{
	while (true)
		planner_pre_calculate();
}
#endif

static void heartBeatTask(void *args)
{
	while (true)
	{
		vTaskDelay(500 / portTICK_PERIOD_MS);
		led_off();
		vTaskDelay(200 / portTICK_PERIOD_MS);
		led_on();
		vTaskDelay(100 / portTICK_PERIOD_MS);
		led_off();
		vTaskDelay(200 / portTICK_PERIOD_MS);
		led_on();
	}
}

static void net_setup(void)
{
	uint8_t mac[6] = {0xC0, 0x00, 0x00, 0x00, 0x00, 0x05};
	uint32_t ip[4] = {10, 55, 2, 2};
	uint32_t ipaddr = (ip[0] << 24) | (ip[1] << 16) | (ip[2] << 8) | (ip[3]);
	ifaceInitialise(mac);
	libip_init(ipaddr, mac);
	ifaceStart();
}

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

void shellSendTask(void *args)
{
	while (true)
	{
		ssize_t len;
		const uint8_t *data = shell_pick_message(&len);
		if (data == NULL)
			continue;

		libip_send_udp_packet(data, len, remote.ip, CONFIG_UDP_PORT, remote.port);
		shell_send_completed();
	}
}

int main(void)
{
	hardware_setup();

#ifdef CONFIG_LIBCORE
	init_steppers();
	planner_lock();
	moves_reset();
#endif

	net_setup();

#ifdef CONFIG_LIBCORE
	xTaskCreate(preCalculateTask, "precalc", configMINIMAL_STACK_SIZE, NULL, 0, NULL);
#endif

	xTaskCreate(shellSendTask, "shell", configMINIMAL_STACK_SIZE, NULL, 0, NULL);
	xTaskCreate(heartBeatTask, "hb", configMINIMAL_STACK_SIZE, NULL, 0, NULL);
	vTaskStartScheduler();

	while (true)
		;

	return 0;
}

/*** ***********************/

void vApplicationTickHook(void)
{
}

void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName)
{
	for (;;)
		;
}

void vApplicationMallocFailedHook(void)
{
	for (;;)
		;
}

void vApplicationIdleHook(void)
{
}

/*** ***********************/
