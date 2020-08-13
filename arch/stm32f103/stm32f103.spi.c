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

#include <stdio.h>
#include <string.h>
#include <output.h>
#include "config.h"

#include <control.h>
#include <moves.h>
#include <planner.h>
#include <gcode_handler.h>

#define FCPU 72000000UL
#define FTIMER 100000UL
#define PSC ((FCPU) / (FTIMER) - 1)
#define TIMEOUT_TIMER_STEP 1000UL

#define min(a, b) ((a) < (b) ? (a) : (b))

static int shell_cfg = 0;


bool configured = false;

void gpio_setup(void);
void step_timer_setup(void);
void usart_setup(int baudrate);
void uart_send(const uint8_t *data, ssize_t len);
void uart_pause(void);
void uart_continue(void);

// ********************************************

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

    /* Enable DMA1 */
    rcc_periph_clock_enable(RCC_DMA1);
}

static void dma_int_enable(void)
{
    /* SPI2 RX on DMA1 Channel 4 */
    nvic_set_priority(NVIC_DMA1_CHANNEL4_IRQ, 0);
    nvic_enable_irq(NVIC_DMA1_CHANNEL4_IRQ);
    /* SPI2 TX on DMA1 Channel 5 */
    nvic_set_priority(NVIC_DMA1_CHANNEL5_IRQ, 0);
    nvic_enable_irq(NVIC_DMA1_CHANNEL5_IRQ);
}

/* Setup SPI */

void spi_setup(void)
{
    int i;
    nvic_enable_irq(NVIC_SPI2_IRQ);

    spi_reset(SPI2);
    for (i = 0; i < 10000; i++)
        asm("");

    spi_set_slave_mode(SPI2);
    spi_disable_software_slave_management(SPI2);

    // SCK - input
    gpio_set_mode(GPIOB, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, GPIO_SPI2_SCK);
    // NSS - input
    gpio_set_mode(GPIOB, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, GPIO_SPI2_NSS);
    // MOSI - input
    gpio_set_mode(GPIOB, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, GPIO_SPI2_MOSI);
    // MISO - output
    gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_SPI2_MISO);

    spi_enable_rx_buffer_not_empty_interrupt(SPI2);

    // interrupt pin
    gpio_set(GPIOB, GPIO11);
    gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_10_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO11);

    spi_enable(SPI2);
}

int blink = 0;
int do_blink(void)
{
    if (blink)
    {
        gpio_set(GPIOC, GPIO13);
        blink = 0;
    }
    else
    {
        gpio_clear(GPIOC, GPIO13);
        blink = 1;
    }
    return 0;
}

/******** SHELL ******************/
#define MNUM 8
#define MLEN 80
char rcv_message[MLEN];
int  rcv_pos = 0;
int  rcv_len = 0;

char messages[MNUM][MLEN];
int num = 0;
int save_id = 0, save_pos = 0, save_len = 0;
int send_id = 0, send_pos = 0, send_len = 0;
bool sending = false;
bool receiving = false;

static void send_interrupt(bool hasdata)
{
    if (hasdata)
        gpio_clear(GPIOB, GPIO11);
    else
        gpio_set(GPIOB, GPIO11);
}

void spi2_isr(void)
{
    if (SPI2_SR & SPI_SR_RXNE)
    {
        char rcv_char = spi_read(SPI2);
        if (!receiving && rcv_char == 0xFF)
        {
            receiving = true;
        }
        if (receiving)
        {
            rcv_message[rcv_pos] = rcv_char;
            rcv_pos++;
            if (rcv_pos == 3)
            {
                rcv_len = ((int)rcv_message[1]) << 8 | rcv_message[2];
            }
            if (rcv_pos == rcv_len + 3)
            {
                receiving = false;
                rcv_pos = 0;
                if (rcv_len > 0)
                    execute_g_command(rcv_message + 3, rcv_len);
            }
        }
    }

    if (SPI2_SR & SPI_SR_TXE)
    {
        if (num > 0)
        {
            if (sending == false)
            {
                send_len = ((int)messages[send_id][1]) << 8 | messages[send_id][2];
                sending = true;
            }

            char send_char = messages[send_id][send_pos];
            send_pos++;
            if (send_pos == send_len + 3)
            {
                send_pos = 0;
                send_id = (send_id + 1) % MNUM;
                num--;
                if (num > 0)
                {
                    send_len = ((int)messages[send_id][1]) << 8 | messages[send_id][2];
                }
                else
                {
                    send_len = 0;
                    sending = false;
                    send_interrupt(false);
                }
            }
            while (SPI2_SR & SPI_SR_BSY)
                ;
            spi_write(SPI2, send_char);
        }
        else
        {
            spi_write(SPI2, 0);
        }
    }
}

ssize_t write_fun(int fd, const void *data, ssize_t len)
{
    int i;
    if (len < 0)
        len = strlen((const char *)data);
    if (fd == 0)
    {
        if (num == MNUM)
            return -1;
        
        len = min(len, MLEN-1);
        messages[save_id][0] = 0xFF;
        messages[save_id][1] = (len >> 8) & 0xFF;
        messages[save_id][2] = len & 0xFF;
        memcpy(messages[save_id] + 3, data, len);
        save_id = (save_id + 1) % MNUM;
        if (num == 0)
            send_interrupt(true);
        num++;
    }
    else
    {
        uart_send(data, len);
    }
    return 0;
}


static void shell_setup(void)
{
    output_set_write_fun(write_fun);
    output_control_set_fd(0);
    output_shell_set_fd(1);
}

/************* END SHELL ****************/

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

void hardware_setup(void)
{

    SCB_VTOR = (uint32_t) 0x08000000;

    clock_setup();
    gpio_setup();
    dma_int_enable();
    spi_setup();
    
    step_timer_setup();
    usart_setup(SHELL_BAUDRATE);
    gpio_set(GPIOC, GPIO13);
    shell_setup();

    uart_send("configured", -1);
    configured = true;
}

void hardware_loop(void)
{
}

