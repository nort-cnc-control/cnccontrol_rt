
#define STM32F1

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/scb.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/exti.h>

#include <stdio.h>
#include <string.h>

#include "config.h"

#include <output/output.h>

static unsigned char txbuf[1024] = {};
static int txpos = 0;
static int txlast = 0;
static bool paused = false;
static bool busy = false;

static void transmit_char(unsigned char data)
{
    busy = true;
    USART_DR(USART1) = data;
}

void usart1_isr(void)
{
    static unsigned char rcvbuf[128];
    static size_t rcvlen = 0;

    if (USART_SR(USART1) & USART_SR_TC)
    {
        busy = false;
        USART_SR(USART1) &= ~USART_SR_TC;
        /* byte has been transmitted */
        if (txpos != txlast && !paused)
        {
            int cur = txpos;
            txpos = (txpos + 1) % sizeof(txbuf);
            transmit_char(txbuf[cur]);
        }
    }

    /* Check if we were called because of RXNE. */
    if (USART_SR(USART1) & USART_SR_RXNE)
    {
        USART_SR(USART1) &= ~USART_SR_RXNE;

        /* Retrieve the data from the peripheral. */
        unsigned char data = usart_recv(USART1);
    }
}

void uart_send(const uint8_t *data, ssize_t len)
{
    int i;
    if (len < 0)
        len = strlen(data);
    bool empty = (txlast == txpos);
    for (i = 0; i < len; i++)
    {
        txbuf[txlast] = ((const char *)data)[i];
        txlast = (txlast + 1) % sizeof(txbuf);
    }
    if (empty && !paused)
    {
        int cur = txpos;
        txpos = (txpos + 1) % sizeof(txbuf);
        transmit_char(txbuf[cur]);
    }
}

void uart_pause(void)
{
    paused = true;
    while (busy)
        ;
}

void uart_continue(void)
{
    paused = false;
    bool empty = (txlast == txpos);
    if (empty)
    {
        int cur = txpos;
        txpos = (txpos + 1) % sizeof(txbuf);
        transmit_char(txbuf[cur]);
    }
}

void usart_setup(int baudrate)
{
    nvic_enable_irq(NVIC_USART1_IRQ);

    /* Setup GPIO pin GPIO_USART1_RE_RX on GPIO port A for receive. */
    gpio_set_mode(GPIOA, GPIO_MODE_INPUT,
                  GPIO_CNF_INPUT_FLOAT, GPIO_USART1_RX);

    /* Setup GPIO pin GPIO_USART1_TX. */
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_10_MHZ,
                  GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_USART1_TX);

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

