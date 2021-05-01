#define STM32F1

#include "platform.h"
#include "net.h"

#include <shell.h>

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
    int i;
    for (i = 0; i < 100000; i++)
        __asm__("nop");
}

static void gpio_setup(void)
{
    /* Blink led */
    gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_10_MHZ,
                  GPIO_CNF_OUTPUT_PUSHPULL, GPIO13);
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

    shell_setup(NULL, uart_send);

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

#ifdef CONFIG_PROTECT_STACK
void __wrap___stack_chk_fail(void)
{
    static int i;
    while (1)
    {
        for (i = 0; i < 0x40000; i++)
            __asm__("nop");
        gpio_set(GPIOC, GPIO13);
        for (i = 0; i < 0x40000; i++)
            __asm__("nop");
        gpio_clear(GPIOC, GPIO13);
    }
}
#endif

void hard_fault_handler(void)
{
    static int i;
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

