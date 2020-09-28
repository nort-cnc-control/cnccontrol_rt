#define STM32F1

#include "platform.h"
#include "shell.h"
#include "net.h"

#ifdef CONFIG_SPI
#include "spi.h"
#endif

#ifdef CONFIG_LIBCORE
#include "steppers.h"
#endif

#ifdef CONFIG_UART
#include "uart.h"
#endif

#ifdef CONFIG_ETHERNET_DEVICE_ENC28J60
#include <enc28j60.h>
#endif

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/scb.h>


#ifdef CONFIG_ETHERNET_DEVICE_ENC28J60
static struct enc28j60_state_s state;
#endif

/* RCC */
static void clock_setup(void)
{
    rcc_clock_setup_in_hse_8mhz_out_72mhz();

    /* Enable GPIOA clock. */
    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_GPIOB);
    rcc_periph_clock_enable(RCC_GPIOC);

    /* Enable clocks for GPIO port A (for GPIO_USART1_TX) and USART1. */
    rcc_periph_clock_enable(RCC_USART1);

    /* Enable TIM2 clock for steps*/
    rcc_periph_clock_enable(RCC_TIM2);

    /* Enable TIM3 clock for connection timeouts */
    rcc_periph_clock_enable(RCC_TIM3);

#ifdef CONFIG_SPI
    /* Enable SPI2 */
    rcc_periph_clock_enable(RCC_AFIO);
    rcc_periph_clock_enable(RCC_SPI2);

    /* Enable DMA1 */
    rcc_periph_clock_enable(RCC_DMA1);
#endif

    // Delay
    static volatile int i;
    for (i = 0; i < 100000; i++)
        ;
}

static void gpio_setup(void)
{
    /* Blink led */
    gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_10_MHZ,
                  GPIO_CNF_OUTPUT_PUSHPULL, GPIO13);
}

static void packet_received(const char *data, size_t len)
{
    shell_data_received(data, len);
    shell_data_completed();
}

static void eth_hard_reset(bool rst)
{
#ifdef CONFIG_ETHERNET_DEVICE_ENC28J60
    if (rst)
        gpio_clear(GPIOB, GPIO11);
    else
        gpio_set(GPIOB, GPIO11);
#endif
}

void hardware_setup(void)
{
    SCB_VTOR = (uint32_t) 0x08000000;

    clock_setup();
    gpio_setup();

#ifdef CONFIG_SPI
    spi_setup();
#endif

#ifdef CONFIG_UART
    uart_setup(9600);
#endif

#ifdef CONFIG_LIBCORE
    steppers_setup();
#endif

#ifdef CONFIG_ETHERNET_DEVICE_ENC28J60
    /* enc28j60 INT pin */
    gpio_set_mode(GPIOB, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, GPIO10);
    gpio_set(GPIOB, GPIO10);

    /* enc28j60 RST pin */
    gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO11);
    gpio_set(GPIOB, GPIO11);

    enc28j60_init(&state, eth_hard_reset, spi_rw, spi_cs, spi_write_buf, spi_read_buf);
#endif

    net_setup(&state, shell_pick_message, shell_send_completed, packet_received);

    shell_setup(NULL);

    gpio_set(GPIOC, GPIO13);

    int i;
    for (i = 0; i < 0x400000; i++)
	__asm__("nop");
}

void hardware_loop(void)
{
    net_receive();
    net_send();
}

void hard_fault_handler(void)
{
    int i;
    while (1)
    {
        for (i = 0; i < 0x400000; i++)
            __asm__("nop");
        gpio_set(GPIOC, GPIO13);
        for (i = 0; i < 0x400000; i++)
            __asm__("nop");
        gpio_clear(GPIOC, GPIO13);
    }
}

