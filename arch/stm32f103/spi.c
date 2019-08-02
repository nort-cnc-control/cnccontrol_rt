#define STM32F1

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/scb.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/exti.h>
#include <libopencm3/stm32/dma.h>

#include <stdint.h>
#include <unistd.h>

uint8_t spi_rw(uint8_t d)
{
    spi_send(SPI2, d);
    return spi_read(SPI2);
}

void spi_cs(uint8_t v)
{
    if (v)
        gpio_set(GPIOB, GPIO_SPI2_NSS);
    else
        gpio_clear(GPIOB, GPIO_SPI2_NSS);
}

static bool send_completed;

static void spi_write_buf_dma(const uint8_t *data, size_t len)
{
    
    /* Reset DMA channels*/
	dma_channel_reset(DMA1, DMA_CHANNEL4);
    dma_channel_reset(DMA1, DMA_CHANNEL5);

    while (!(SPI_SR(SPI2) & SPI_SR_TXE))
        ;

    volatile uint8_t temp_data __attribute__ ((unused));
	while (SPI_SR(SPI2) & (SPI_SR_RXNE | SPI_SR_OVR))
    {
		temp_data = SPI_DR(SPI2);
    }

    // use SPI2 register for write
    dma_set_peripheral_address(DMA1, DMA_CHANNEL5, SPI_DR(SPI2));
	// buffer to send
    dma_set_memory_address(DMA1, DMA_CHANNEL5, (uint32_t)data);
    dma_set_number_of_data(DMA1, DMA_CHANNEL5, len);

    // read from memory
	dma_set_read_from_memory(DMA1, DMA_CHANNEL5);
    dma_enable_memory_increment_mode(DMA1, DMA_CHANNEL5);

    // send bytes
	dma_set_peripheral_size(DMA1, DMA_CHANNEL5, DMA_CCR_PSIZE_8BIT);
	dma_set_memory_size(DMA1, DMA_CHANNEL5, DMA_CCR_MSIZE_8BIT);

    // select priority    
    dma_set_priority(DMA1, DMA_CHANNEL5, DMA_CCR_PL_HIGH);

    // allow interrupts
    dma_enable_transfer_complete_interrupt(DMA1, DMA_CHANNEL5);
    dma_enable_transfer_error_interrupt(DMA1, DMA_CHANNEL5);

    send_completed = false;
    spi_enable_tx_dma(SPI2);

    dma_enable_channel(DMA1, DMA_CHANNEL5);

    while (!send_completed)
        ;
}

void dma1_channel5_isr(void)
{
    if (DMA1_ISR & DMA_ISR_TCIF5)
    {
    	DMA1_IFCR |= DMA_IFCR_CTCIF5;
	}
    if (DMA1_ISR & DMA_ISR_TEIF5)
    {
        uart_send("dma err", -1);
    	DMA1_IFCR |= DMA_IFCR_CTEIF5;
    }

	dma_disable_transfer_complete_interrupt(DMA1, DMA_CHANNEL5);
	dma_disable_transfer_error_interrupt(DMA1, DMA_CHANNEL5);

    spi_disable_tx_dma(SPI2);
	dma_disable_channel(DMA1, DMA_CHANNEL5);
    
    send_completed = true;
}

static void spi_write_buf_manual(const uint8_t *data, size_t len)
{
    while (len-- > 0)
    {
        spi_rw(*(data++));
    }
}

void spi_write_buf(const uint8_t *data, size_t len)
{
    //if (len < 16)
        spi_write_buf_manual(data, len);
    //else
    //    spi_write_buf_dma(data, len);
}

static bool read_completed;

static void spi_read_buf_dma(uint8_t *data, size_t len)
{
    
    /* Reset DMA channels*/
	dma_channel_reset(DMA1, DMA_CHANNEL4);
    dma_channel_reset(DMA1, DMA_CHANNEL5);

    while (!(SPI_SR(SPI2) & SPI_SR_TXE))
        ;

    volatile uint8_t temp_data __attribute__ ((unused));
	while (SPI_SR(SPI2) & (SPI_SR_RXNE | SPI_SR_OVR))
    {
		temp_data = SPI_DR(SPI2);
    }

    // use SPI2 register for write
    dma_set_peripheral_address(DMA1, DMA_CHANNEL4, SPI_DR(SPI2));

    // buffer to send
    dma_set_memory_address(DMA1, DMA_CHANNEL4, (uint32_t)data);
    dma_set_number_of_data(DMA1, DMA_CHANNEL4, len);

    // read to memory
	dma_set_read_from_peripheral(DMA1, DMA_CHANNEL4);
    dma_enable_memory_increment_mode(DMA1, DMA_CHANNEL4);

    // read bytes
	dma_set_peripheral_size(DMA1, DMA_CHANNEL4, DMA_CCR_PSIZE_8BIT);
	dma_set_memory_size(DMA1, DMA_CHANNEL4, DMA_CCR_MSIZE_8BIT);

    // select priority    
    dma_set_priority(DMA1, DMA_CHANNEL4, DMA_CCR_PL_HIGH);

    // allow interrupts
    dma_enable_transfer_complete_interrupt(DMA1, DMA_CHANNEL4);
    dma_enable_transfer_error_interrupt(DMA1, DMA_CHANNEL4);

    read_completed = false;
    spi_enable_rx_dma(SPI2);

    dma_enable_channel(DMA1, DMA_CHANNEL4);

    while (!read_completed)
        ;
}

void dma1_channel4_isr(void)
{
    if (DMA1_ISR & DMA_ISR_TCIF4)
    {
    	DMA1_IFCR |= DMA_IFCR_CTCIF4;
	}
    if (DMA1_ISR & DMA_ISR_TEIF4)
    {
        uart_send("dma err", -1);
    	DMA1_IFCR |= DMA_IFCR_CTEIF4;
    }

	dma_disable_transfer_complete_interrupt(DMA1, DMA_CHANNEL4);
	dma_disable_transfer_error_interrupt(DMA1, DMA_CHANNEL4);

    spi_disable_rx_dma(SPI2);
	dma_disable_channel(DMA1, DMA_CHANNEL4);

    read_completed = true;
}

static void spi_read_buf_manual(uint8_t *data, size_t len)
{
    while (len > 0)
    {
        *(data++) = spi_rw(0xFF);
        len--;
    }
}

void spi_read_buf(uint8_t *data, size_t len)
{
    //if (len < 16)
        spi_read_buf_manual(data, len);
    //else
    //    spi_read_buf_dma(data, len);
}

void spi_setup(void)
{
    nvic_enable_irq(NVIC_SPI2_IRQ);

    gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_SPI2_MOSI | GPIO_SPI2_SCK);
    gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO_SPI2_NSS);
    gpio_set_mode(GPIOB, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, GPIO_SPI2_MISO);

    spi_reset(SPI2);

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
