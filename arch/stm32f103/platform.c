#define STM32F1

#include "platform.h"
#include "steppers.h"
#include "spi.h"
#include "shell.h"
#include "net.h"

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

    /* Enable SPI2 */
    rcc_periph_clock_enable(RCC_AFIO);
    rcc_periph_clock_enable(RCC_SPI2);

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

void hardware_setup(void)
{
    SCB_VTOR = (uint32_t) 0x08000000;

    clock_setup();
    spi_setup();
    gpio_setup();
    steppers_setup();

    net_setup(shell_send_completed, packet_received);
    shell_setup(NULL, net_send);

    gpio_set(GPIOC, GPIO13);

    int i;
    for (i = 0; i < 0x400000; i++)
	__asm__("nop");
}

void hardware_loop(void)
{
    net_receive();
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

