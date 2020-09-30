
#define STM32F1

#include <libopencm3/stm32/dma.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/cm3/nvic.h>

#include <stdio.h>
#include <string.h>

#include "config.h"

static volatile int i;
void uart_send(const uint8_t *data, size_t len)
{
    gpio_clear(GPIOA, GPIO12);
    for (i = 0; i < 100000; i++)
        __asm__("nop");
    while (len > 0)
    {
        uint16_t d = *(data++);
        while ((USART_SR(USART1) & USART_SR_TXE) == 0)
            ;
        USART_DR(USART1) = d;
//        usart_send_blocking(USART1, d);
        len--;
    }
    for (i = 0; i < 100000; i++)
        __asm__("nop");
    gpio_set(GPIOA, GPIO12);
}

void uart_setup(int baudrate)
{
//    nvic_enable_irq(NVIC_USART1_IRQ);

    /* Setup GPIO pin GPIO_USART1_RE_RX on GPIO port A for receive. */
    gpio_set_mode(GPIOA, GPIO_MODE_INPUT,
                  GPIO_CNF_INPUT_FLOAT, GPIO_USART1_RX);

    /* Setup GPIO pin GPIO_USART1_TX. */
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_10_MHZ,
                  GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_USART1_TX);

    /* Setup GPIO pin GPIO_USART1_RTS. */
    gpio_set(GPIOA, GPIO12);
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
                  GPIO_CNF_OUTPUT_PUSHPULL, GPIO12);

    /* Setup UART parameters. */
    usart_set_baudrate(USART1, baudrate);
    usart_set_databits(USART1, 8);
    usart_set_stopbits(USART1, USART_STOPBITS_1);
    usart_set_mode(USART1, USART_MODE_TX_RX);
    usart_set_parity(USART1, USART_PARITY_NONE);
    usart_set_flow_control(USART1, USART_FLOWCONTROL_NONE);

    USART_CR1(USART1) |= USART_CR1_RXNEIE;
    USART_CR1(USART1) |= USART_CR1_TCIE;

    /* Finally enable the USART. */
    usart_enable(USART1);
}

