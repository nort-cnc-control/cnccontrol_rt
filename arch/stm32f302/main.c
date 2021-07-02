#include "config.h"

#ifdef CONFIG_LIBCORE
#include <control/moves/moves.h>
#include <control/planner/planner.h>
#include <control/control.h>
#include <output/output.h>
#include "steppers.h"
#endif

#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

#include <shell.h>
#include <iface.h>

#include <FreeRTOS_IP.h>
#include <FreeRTOS_Sockets.h>
#include <FreeRTOS_TCP_IP.h>
#include <FreeRTOS_UDP_IP.h>


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

static uint32_t ip;
static uint32_t netmask;
static uint32_t gateway;
static uint32_t dns;
static uint32_t broadcast;

static void net_setup(void)
{
	const uint8_t ucIPAddress[ ipIP_ADDRESS_LENGTH_BYTES ]        = {10,55,1,120};
	const uint8_t ucNetMask[ ipIP_ADDRESS_LENGTH_BYTES ]          = {255,255,255,0};
	const uint8_t ucGatewayAddress[ ipIP_ADDRESS_LENGTH_BYTES ]   = {10,55,1,1};
	const uint8_t ucDNSServerAddress[ ipIP_ADDRESS_LENGTH_BYTES ] = {10,55,1,1};
	const uint8_t ucMACAddress[ ipMAC_ADDRESS_LENGTH_BYTES ]      = {0x0C, 0x00, 0x00, 0x00, 0x00, 0x02};
	memcpy(eth_mac, ucMACAddress, ipMAC_ADDRESS_LENGTH_BYTES);

	FreeRTOS_IPInit(ucIPAddress, ucNetMask, ucGatewayAddress, ucDNSServerAddress, ucMACAddress);
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
		// send data

		// mark data as sended
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

const char *pcApplicationHostnameHook( void )
{
    return "cnccontrol_rt";
}


void vApplicationIPNetworkEventHook( eIPCallbackEvent_t eNetworkEvent )
{
	static bool started = false;
	if( eNetworkEvent == eNetworkUp )
	{
		FreeRTOS_GetAddressConfiguration(&ip, &netmask, &gateway, &dns);
		broadcast = ip | ~netmask;
		if (!started)
		{
			started = true;
			xTaskCreate(shellSendTask, "shell", configMINIMAL_STACK_SIZE, NULL, 0, NULL);
		}
	}
}

/*** ***********************/

