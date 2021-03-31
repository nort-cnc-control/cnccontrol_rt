#define STM32F3

#include <libopencm3/stm32/dma.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/cm3/nvic.h>

#include <stdio.h>
#include <string.h>

#include "config.h"

#define UART_ID   USART1
#define UART_PORT GPIOA
#define UART_TX   GPIO9
#define UART_RX   GPIO10
#define UART_RTS  GPIO12

static volatile int i;
void uart_send(const uint8_t *data, size_t len)
{
    gpio_clear(UART_PORT, UART_RTS);
    for (i = 0; i < 100000; i++)
        __asm__("nop");
    while (len > 0)
    {
        uint16_t d = *(data++);
        while ((USART_ISR(UART_ID) & USART_ISR_TXE) == 0)
            ;
        USART_TDR(UART_ID) = d;
//        usart_send_blocking(USART1, d);
        len--;
    }
    for (i = 0; i < 100000; i++)
        __asm__("nop");
    gpio_set(UART_PORT, UART_RTS);
}

void uart_setup(int baudrate)
{
//    nvic_enable_irq(NVIC_USART1_IRQ);

    /* Setup GPIO pin GPIO_USART1_RE_RX on GPIO port A for receive. */
    gpio_mode_setup(UART_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, UART_RX | UART_TX);
    gpio_set_af(UART_PORT, GPIO_AF7, UART_RX | UART_TX);

    /* Setup GPIO pin GPIO_USART1_RTS. */
    gpio_mode_setup(UART_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, UART_RTS);
    gpio_set_output_options(UART_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, UART_RTS);
    gpio_set(UART_PORT, UART_RTS);

    /* Setup UART parameters. */
    usart_set_baudrate(UART_ID, baudrate);
    usart_set_databits(UART_ID, 8);
    usart_set_stopbits(UART_ID, USART_STOPBITS_1);
    usart_set_mode(UART_ID, USART_MODE_TX_RX);
    usart_set_parity(UART_ID, USART_PARITY_NONE);
    usart_set_flow_control(UART_ID, USART_FLOWCONTROL_NONE);

    USART_CR1(UART_ID) |= USART_CR1_RXNEIE;
    USART_CR1(UART_ID) |= USART_CR1_TCIE;

    /* Finally enable the USART. */
    usart_enable(UART_ID);
}
