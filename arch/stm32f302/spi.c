#define STM32F3

#include <stdbool.h>

#include "spi.h"

#include <libopencm3/stm32/dma.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/spi.h>

#define SPI_ENABLE_DMA false
#define SPI_DMA_MINIMAL 16

/* SPI */

#define SPI_ID    SPI2
#define SPI_PORT  GPIOB
#define SPI_NSS   GPIO12
#define SPI_SCK   GPIO13
#define SPI_MISO  GPIO14
#define SPI_MOSI  GPIO15

uint8_t spi_rw(uint8_t data)
{
    spi_send(SPI_ID, data);
    uint8_t c = spi_read(SPI_ID);
    return c;
}

void spi_cs(bool enable)
{
    if (enable)
        gpio_clear(SPI_PORT, SPI_NSS);
    else
        gpio_set(SPI_PORT, SPI_NSS);
}

static volatile bool txcomplete, rxcomplete;

void spi_write_buf(const uint8_t *data, size_t len)
{
    if (SPI_ENABLE_DMA && len >= SPI_DMA_MINIMAL)
    {
        txcomplete = false;

        dma_set_memory_address(DMA1, DMA_CHANNEL5, (uint32_t)data);
        dma_set_number_of_data(DMA1, DMA_CHANNEL5, len);
        dma_enable_channel(DMA1, DMA_CHANNEL5);
        spi_cs(true);
        spi_enable_tx_dma(SPI_ID);

        while (!txcomplete)
            ;
        spi_cs(false);
    }
    else
    {
        while (len-- > 0)
        {
            while (!(SPI_SR(SPI_ID) & SPI_SR_TXE))
               ;
            SPI_DR(SPI_ID) = *(data++);
        }
        while (SPI_SR(SPI_ID) & SPI_SR_BSY)
            ;
    }

    char c = SPI_DR(SPI_ID);
    c = SPI_SR(SPI_ID);
}

void spi_read_buf(uint8_t *data, size_t len)
{
    if (SPI_ENABLE_DMA && len >= SPI_DMA_MINIMAL)
    {
        rxcomplete = false;

        dma_set_memory_address(DMA1, DMA_CHANNEL4, (uint32_t)data);
        dma_set_number_of_data(DMA1, DMA_CHANNEL4, len);
        dma_enable_channel(DMA1, DMA_CHANNEL4);
        spi_cs(true);
        spi_enable_rx_dma(SPI_ID);

        while (!rxcomplete)
            ;
        spi_cs(false);
    }
    else
    {
        while (len > 0)
        {
            SPI_DR(SPI_ID) = 0xFF;

            while (!(SPI_SR(SPI_ID) & SPI_SR_RXNE))
                ;
            uint8_t c = SPI_DR(SPI_ID);

            *(data++) = c;
            len--;
        }
        while (SPI_SR(SPI_ID) & SPI_SR_BSY)
            ;
    }
}

void dma1_channel4_isr(void)
{
    if (dma_get_interrupt_flag(DMA1, DMA_CHANNEL4, DMA_TCIF))
    {
        dma_clear_interrupt_flags(DMA1, DMA_CHANNEL5, DMA_TCIF);
        while (SPI_SR(SPI_ID) & SPI_SR_BSY)
            ;
        spi_disable_rx_dma(SPI2);
        dma_disable_channel(DMA1, DMA_CHANNEL4);

        rxcomplete = true;
    }
}

void dma1_channel5_isr(void)
{
    if (dma_get_interrupt_flag(DMA1, DMA_CHANNEL5, DMA_TCIF))
    {
        dma_clear_interrupt_flags(DMA1, DMA_CHANNEL5, DMA_TCIF);
        while (SPI_SR(SPI2) & SPI_SR_BSY)
            ;
        spi_disable_tx_dma(SPI_ID);
        dma_disable_channel(DMA1, DMA_CHANNEL5);

        txcomplete = true;
    }
}

static void spi_dma_setup(void)
{
    dma_channel_reset(DMA1, DMA_CHANNEL5);

    static volatile int i;
    for (i = 0; i < 100000; i++)
        ;

    dma_set_peripheral_address(DMA1, DMA_CHANNEL5, (uint32_t)&SPI2_DR);
    dma_set_read_from_memory(DMA1, DMA_CHANNEL5);
    dma_enable_memory_increment_mode(DMA1, DMA_CHANNEL5);
    dma_disable_peripheral_increment_mode(DMA1, DMA_CHANNEL5);
    dma_set_peripheral_size(DMA1, DMA_CHANNEL5, DMA_CCR_PSIZE_8BIT);
    dma_set_memory_size(DMA1, DMA_CHANNEL5, DMA_CCR_MSIZE_8BIT);
    dma_set_priority(DMA1, DMA_CHANNEL5, DMA_CCR_PL_LOW);
    dma_enable_transfer_complete_interrupt(DMA1, DMA_CHANNEL5);

    nvic_set_priority(NVIC_DMA1_CHANNEL5_IRQ, 0);
    nvic_enable_irq(NVIC_DMA1_CHANNEL5_IRQ);
}

void spi_setup(void)
{
//    nvic_enable_irq(NVIC_SPI2_IRQ);
    gpio_mode_setup(SPI_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLDOWN, SPI_MOSI | SPI_SCK | SPI_MISO);
    gpio_set_af(SPI_PORT, GPIO_AF5, SPI_SCK | SPI_MISO | SPI_MOSI);
    gpio_set_output_options(SPI_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, SPI_SCK | SPI_MOSI);

    gpio_mode_setup(SPI_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, SPI_NSS);
    gpio_set_output_options(SPI_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, SPI_NSS);
    gpio_set(SPI_PORT, SPI_NSS);

    spi_reset(SPI_ID);

    static volatile int i;
    for (i = 0; i < 100000; i++)
        ;

    spi_init_master(SPI_ID,
                    SPI_CR1_BAUDRATE_FPCLK_DIV_16,
                    SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,
                    SPI_CR1_CPHA_CLK_TRANSITION_1,
                    SPI_CR1_MSBFIRST);

    spi_enable_software_slave_management(SPI_ID);
    spi_set_nss_high(SPI_ID);
    spi_dma_setup();
    spi_enable(SPI_ID);
}

