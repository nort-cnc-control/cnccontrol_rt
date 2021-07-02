#define STM32F3

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "iface.h"
#include <enc28j60.h>

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/dma.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/spi.h>

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>

#include <FreeRTOS_IP.h>
#include <FreeRTOS_IP_Private.h>
#include <FreeRTOS_TCP_IP.h>
#include <FreeRTOS_UDP_IP.h>
#include <NetworkBufferManagement.h>


#define SPI_ENABLE_DMA false
#define SPI_DMA_MINIMAL 16

/* SPI */

#define SPI_RCC RCC_SPI2
#define SPI_ID SPI2
#define SPI_PORT GPIOB
#define SPI_NSS GPIO12
#define SPI_SCK GPIO13
#define SPI_MISO GPIO14
#define SPI_MOSI GPIO15

/* Ethernet */

#define ETH_RST_PORT GPIOB
#define ETH_RST_PIN GPIO11

uint8_t spi_rw(uint8_t data)
{
	spi_send8(SPI_ID, data);
	uint8_t c = spi_read8(SPI_ID);
	return c;
}

void spi_cs(bool enable)
{
	if (enable)
	{
		//spi_set_nss_low(SPI_ID);
		gpio_clear(SPI_PORT, SPI_NSS);
	}
	else
	{
		//spi_set_nss_high(SPI_ID);
		gpio_set(SPI_PORT, SPI_NSS);
	}
}

void spi_write_buf(const uint8_t *data, size_t len)
{
	while (len-- > 0)
	{
		spi_rw(*(data++));
	}
}

void spi_read_buf(uint8_t *data, size_t len)
{
	while (len-- > 0)
	{
		*(data++) = spi_rw(0xFF);
	}
}

void spi_setup(void)
{
	int i;
	/* Enable SPI2 */
	rcc_periph_clock_enable(SPI_RCC);

	for (i = 0; i < 100000; i++)
		asm("nop");

	gpio_mode_setup(SPI_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, SPI_MOSI | SPI_SCK | SPI_MISO);
	gpio_set_af(SPI_PORT, GPIO_AF5, SPI_SCK | SPI_MISO | SPI_MOSI);
	gpio_set_output_options(SPI_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, SPI_SCK | SPI_MOSI);

	gpio_mode_setup(SPI_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, SPI_NSS);
	gpio_set_output_options(SPI_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, SPI_NSS);
	gpio_set(SPI_PORT, SPI_NSS);

	spi_set_master_mode(SPI_ID);
	spi_set_baudrate_prescaler(SPI_ID, SPI_CR1_BR_FPCLK_DIV_64);
	spi_set_clock_polarity_0(SPI_ID);
	spi_set_clock_phase_0(SPI_ID);
	spi_set_full_duplex_mode(SPI_ID);
	spi_set_unidirectional_mode(SPI_ID); /* bidirectional but in 3-wire */
	spi_set_data_size(SPI_ID, SPI_CR2_DS_8BIT);
	spi_enable_software_slave_management(SPI_ID);
	spi_send_msb_first(SPI_ID);
	spi_set_nss_high(SPI_ID);
	//spi_enable_ss_output(SPI_ID);
	spi_fifo_reception_threshold_8bit(SPI_ID);
	SPI_I2SCFGR(SPI_ID) &= ~SPI_I2SCFGR_I2SMOD;
	spi_enable(SPI_ID);
}

/* Network */
static xSemaphoreHandle ethMutex = NULL;
static struct enc28j60_state_s eth_state;
uint8_t eth_mac[6] = { 0x0C, 0x00, 0x00, 0x00, 0x00, 0x02 };

static void eth_hard_reset(bool rst)
{
	if (rst)
		gpio_clear(ETH_RST_PORT, ETH_RST_PIN);
	else
		gpio_set(ETH_RST_PORT, ETH_RST_PIN);
}

BaseType_t xNetworkInterfaceOutput(NetworkBufferDescriptor_t* const pxNetworkBuffer,
                                   BaseType_t xReleaseAfterSend)
{
	BaseType_t result = pdFAIL;

	if (xSemaphoreTake(ethMutex, portMAX_DELAY) == pdTRUE)
	{
		enc28j60_send_data(&eth_state, pxNetworkBuffer->pucEthernetBuffer, pxNetworkBuffer->xDataLength);
		xSemaphoreGive(ethMutex);
		iptraceNETWORK_INTERFACE_TRANSMIT();
		result = pdPASS;
	}

	if(xReleaseAfterSend != pdFALSE)
	{
		vReleaseNetworkBufferAndDescriptor(pxNetworkBuffer);
	}
	return result;
}

static uint8_t eth_buf[1524];
static void netTask(void *arg)
{
	while (true)
	{
		if (!xSemaphoreTake(ethMutex, portMAX_DELAY))
			continue;
		bool received = enc28j60_has_package(&eth_state);
		xSemaphoreGive(ethMutex);

		if (received)
		{
			uint32_t status, crc;

			if (!xSemaphoreTake(ethMutex, portMAX_DELAY))
				continue;
			ssize_t len = enc28j60_read_packet(&eth_state, eth_buf, sizeof(eth_buf), &status, &crc);
			xSemaphoreGive(ethMutex);

			if (len > 0)
			{
				// Send data to stack
				NetworkBufferDescriptor_t *pxBufferDescriptor = pxGetNetworkBufferWithDescriptor(len, portMAX_DELAY);
				if (pxBufferDescriptor == NULL)
				{
					iptraceETHERNET_RX_EVENT_LOST();
					continue;
				}

				memcpy(pxBufferDescriptor->pucEthernetBuffer, eth_buf, len);
				pxBufferDescriptor->xDataLength = len;
				if(eConsiderFrameForProcessing(pxBufferDescriptor->pucEthernetBuffer) == eProcessBuffer)
				{
					IPStackEvent_t xRxEvent;
					xRxEvent.eEventType = eNetworkRxEvent;
					xRxEvent.pvData = (void *) pxBufferDescriptor;

					if (xSendEventStructToIPTask(&xRxEvent, 0) == pdFALSE)
					{
						iptraceETHERNET_RX_EVENT_LOST();
						vReleaseNetworkBufferAndDescriptor(pxBufferDescriptor);
					}
					else
					{
						iptraceNETWORK_INTERFACE_RECEIVE();
					}
				}
				else
				{
					vReleaseNetworkBufferAndDescriptor(pxBufferDescriptor);
				}
			}
		}
	}
}

BaseType_t xNetworkInterfaceInitialise(void)
{
	/* Init SPI2 */
	spi_setup();

	/* Configure hard reset pin */
	gpio_mode_setup(ETH_RST_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, ETH_RST_PIN);
	gpio_set_output_options(ETH_RST_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, ETH_RST_PIN);
	gpio_set(ETH_RST_PORT, ETH_RST_PIN);

	/* Init enc28j60 module */
	taskENTER_CRITICAL();
	enc28j60_init(&eth_state, eth_hard_reset, spi_rw, spi_cs, spi_write_buf, spi_read_buf);
	bool res = enc28j60_configure(&eth_state, eth_mac, 4096, true);
	taskEXIT_CRITICAL();

	ethMutex = xSemaphoreCreateMutex();
	xTaskCreate(netTask, "enc28j60", 2048, NULL, 0, NULL);
}

