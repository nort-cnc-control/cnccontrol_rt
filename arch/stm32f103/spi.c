#define STM32F1

#include "spi.h"

#include <libopencm3/stm32/dma.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/spi.h>

#define SPI_ENABLE_DMA false
#define SPI_DMA_MINIMAL 16

/* SPI */

uint8_t spi_rw(uint8_t data)
{
    spi_send(SPI2, data);
    uint8_t c = spi_read(SPI2);
    return c;
}

void spi_cs(bool enable)
{
    if (enable)
        gpio_clear(GPIOB, GPIO_SPI2_NSS);
    else
        gpio_set(GPIOB, GPIO_SPI2_NSS);
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
        gpio_clear(GPIOB, GPIO12);
        spi_enable_tx_dma(SPI2);

        while (!txcomplete)
            ;
    }
    else
    {
        while (len-- > 0)
        {
            while (!(SPI_SR(SPI2) & SPI_SR_TXE))
               ;
            SPI_DR(SPI2) = *(data++);
        }
        while (SPI_SR(SPI2) & SPI_SR_BSY)
            ;
    }

    char c = SPI_DR(SPI2);
    c = SPI_SR(SPI2);
}

void spi_read_buf(uint8_t *data, size_t len)
{
    if (SPI_ENABLE_DMA && len >= SPI_DMA_MINIMAL)
    {
        rxcomplete = false;

        dma_set_memory_address(DMA1, DMA_CHANNEL4, (uint32_t)data);
        dma_set_number_of_data(DMA1, DMA_CHANNEL4, len);
        dma_enable_channel(DMA1, DMA_CHANNEL4);
        gpio_clear(GPIOB, GPIO12);
        spi_enable_rx_dma(SPI2);

        while (!rxcomplete)
            ;

    }
    else
    {
        while (len > 0)
        {
            SPI_DR(SPI2) = 0xFF;

            while (!(SPI_SR(SPI2) & SPI_SR_RXNE))
                ;
            uint8_t c = SPI_DR(SPI2);

            *(data++) = c;
            len--;
        }
        while (SPI_SR(SPI2) & SPI_SR_BSY)
            ;
    }
}

void dma1_channel4_isr(void)
{
    if (dma_get_interrupt_flag(DMA1, DMA_CHANNEL4, DMA_TCIF))
    {
        dma_clear_interrupt_flags(DMA1, DMA_CHANNEL5, DMA_TCIF);
        while (SPI_SR(SPI2) & SPI_SR_BSY)
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
        spi_disable_tx_dma(SPI2);
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

    gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_SPI2_MOSI | GPIO_SPI2_SCK);
    gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO_SPI2_NSS);
    gpio_set_mode(GPIOB, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, GPIO_SPI2_MISO);
    gpio_set(GPIOB, GPIO_SPI2_MISO);

    spi_reset(SPI2);

    static volatile int i;
    for (i = 0; i < 100000; i++)
        ;

    spi_init_master(SPI2,
                    SPI_CR1_BAUDRATE_FPCLK_DIV_2,
                    SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,
                    SPI_CR1_CPHA_CLK_TRANSITION_1,
                    SPI_CR1_DFF_8BIT,
                    SPI_CR1_MSBFIRST);
    spi_enable_software_slave_management(SPI2);
    spi_set_nss_high(SPI2);

    spi_dma_setup();

    spi_enable(SPI2);
}

