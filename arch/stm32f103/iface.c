#define STM32F1

#include <string.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/rcc.h>

#include <enc28j60.h>
#include "iface.h"
#include <net.h>

#include <shell.h>

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
	spi_send(SPI_ID, data);
	uint8_t c = spi_read(SPI_ID);
	return c;
}

void spi_cs(bool enable)
{
	if (enable)
	{
		gpio_clear(SPI_PORT, SPI_NSS);
	}
	else
	{
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
	rcc_periph_clock_enable(SPI_RCC);

	for (i = 0; i < 100000; i++)
		asm("nop");

    gpio_set_mode(SPI_PORT, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, SPI_MOSI | SPI_SCK);
    gpio_set_mode(SPI_PORT, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, SPI_NSS);
    gpio_set_mode(SPI_PORT, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, SPI_MISO);
    gpio_set(SPI_PORT, SPI_MISO);

    spi_reset(SPI_ID);

    spi_init_master(SPI_ID,
                    SPI_CR1_BAUDRATE_FPCLK_DIV_8,
                    SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,
                    SPI_CR1_CPHA_CLK_TRANSITION_1,
                    SPI_CR1_DFF_8BIT,
                    SPI_CR1_MSBFIRST);
    spi_enable_software_slave_management(SPI_ID);
    spi_set_nss_high(SPI_ID);
    spi_enable(SPI_ID);
}

/* Network */
static struct enc28j60_state_s eth_state;
static uint8_t eth_mac[6];

static void eth_hard_reset(bool rst)
{
	if (rst)
		gpio_clear(ETH_RST_PORT, ETH_RST_PIN);
	else
		gpio_set(ETH_RST_PORT, ETH_RST_PIN);
}

void send_ethernet_frame(const uint8_t *payload, size_t payload_len)
{
	enc28j60_send_data(&eth_state, payload, payload_len);
}

static uint8_t eth_buf[1524];
static void netPoll(void)
{
	bool received = enc28j60_has_package(&eth_state);
		
	if (received)
	{
		uint32_t status, crc;
		ssize_t len = enc28j60_read_packet(&eth_state, eth_buf, sizeof(eth_buf), &status, &crc);

		if (len > 0)
		{
			libip_handle_ethernet(eth_buf, len);
		}
	}
}

void ifaceInitialise(const uint8_t mac[6])
{
	memcpy(eth_mac, mac, 6);
	/* Init SPI2 */
	spi_setup();

	/* Configure hard reset pin */
	gpio_set_mode(ETH_RST_PORT, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, ETH_RST_PIN);
	gpio_set(ETH_RST_PORT, ETH_RST_PIN);

	/* Init enc28j60 module */
	enc28j60_init(&eth_state, eth_hard_reset, spi_rw, spi_cs, spi_write_buf, spi_read_buf);
	bool res = enc28j60_configure(&eth_state, eth_mac, 4096, true);
}

void ifacePoll(void)
{
	netPoll();
}
