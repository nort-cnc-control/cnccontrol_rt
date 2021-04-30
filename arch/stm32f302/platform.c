#define STM32F3

#include "platform.h"
#include "iface.h"

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
    rcc_clock_setup_pll(&rcc_hse8mhz_configs[RCC_CLOCK_HSE8_72MHZ]);

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

    // Delay
    int i;
    for (i = 0; i < 100000; i++)
        __asm__("nop");
}

static void gpio_setup(void)
{
    /* Blink led */
    gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_OTYPE_PP, GPIO13);
}

static void eth_hard_reset(bool rst)
{
    if (rst)
        gpio_clear(GPIOB, GPIO11);
    else
        gpio_set(GPIOB, GPIO11);
}

void hardware_setup(void)
{
    clock_setup();
    gpio_setup();

#ifdef CONFIG_UART
    uart_setup(9600);
#endif

#ifdef CONFIG_LIBCORE
    steppers_setup();
#endif

    gpio_set(GPIOC, GPIO13);

    int i;
    for (i = 0; i < 0x400000; i++)
	    __asm__("nop");
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

void led_on(void)
{
    gpio_set(GPIOC, GPIO13);
}

void led_off(void)
{
    gpio_clear(GPIOC, GPIO13);
}

struct fault_state_s
{
        uint32_t r0;
        uint32_t r1;
        uint32_t r2;
        uint32_t r3;
        uint32_t r12;
        uint32_t lr;
        uint32_t pc;
        uint32_t psr;
};

void hard_fault_handler(void)
{
    struct fault_state_s *stack_ptr;

    asm(
        "TST lr, #4 \n" //Тестируем 3ий бит указателя стека(побитовое И)
        "ITE EQ \n"   //Значение указателя стека имеет бит 3?
        "MRSEQ %[ptr], MSP  \n"  //Да, сохраняем основной указатель стека
        "MRSNE %[ptr], PSP  \n"  //Нет, сохраняем указатель стека процесса
        : [ptr] "=r" (stack_ptr)
    );

    uint8_t *prev_stack = (uint8_t *)stack_ptr - sizeof(struct fault_state_s);

    while (1)
    {
        static int i;
        for (i = 0; i < 0x400000; i++)
            __asm__("nop");
        gpio_set(GPIOC, GPIO13);
        for (i = 0; i < 0x400000; i++)
            __asm__("nop");
        gpio_clear(GPIOC, GPIO13);
    }
}
