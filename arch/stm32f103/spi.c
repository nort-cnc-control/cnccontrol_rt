#define STM32F1

#include "spi.h"

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/spi.h>

/* SPI */

uint8_t spi_rw(uint8_t data)
{
    spi_send(SPI2, data);
    return spi_read(SPI2);
}

void spi_cs(bool enable)
{
    if (enable)
        gpio_clear(GPIOB, GPIO_SPI2_NSS);
    else
        gpio_set(GPIOB, GPIO_SPI2_NSS);
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
    while (len > 0)
    {
        *(data++) = spi_rw(0xFF);
        len--;
    }
}

void spi_setup(void)
{
    nvic_enable_irq(NVIC_SPI2_IRQ);

    gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_SPI2_MOSI | GPIO_SPI2_SCK);
    gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO_SPI2_NSS);
    gpio_set_mode(GPIOB, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, GPIO_SPI2_MISO);
    gpio_set(GPIOB, GPIO_SPI2_MISO);

    spi_reset(SPI2);

    static volatile int i;
    for (i = 0; i < 100000; i++)
        ;

    spi_init_master(SPI2,
                    SPI_CR1_BAUDRATE_FPCLK_DIV_8,
                    SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,
                    SPI_CR1_CPHA_CLK_TRANSITION_1,
                    SPI_CR1_DFF_8BIT,
                    SPI_CR1_MSBFIRST);
    spi_enable_software_slave_management(SPI2);
    spi_set_nss_high(SPI2);

    spi_enable(SPI2);
}

