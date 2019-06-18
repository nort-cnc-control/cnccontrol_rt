#define STM32F1

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/scb.h>
#include <libopencm3/stm32/timer.h>

#include "config.h"

#include <serial_io.h>
#include <shell_print.h>
#include <shell_read.h>

#include <control.h>
#include <moves.h>
#include <planner.h>
#include <gcode_handler.h>

#define FCPU 72000000UL
#define FTIMER 100000UL
#define PSC ((FCPU) / (FTIMER) - 1)


// *************** CONFIGURABLE PART **********
static struct serial_cbs_s *serial_cbs = &serial_io_serial_cbs;
static struct shell_cbs_s  *shell_cbs =  &serial_io_shell_cbs;
static void (*init_serial_shell)(void) = serial_io_init;
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

    /* Enable TIM2 clock */
    rcc_periph_clock_enable(RCC_TIM2);
}

static void usart_setup(int baudrate)
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

static void step_timer_setup(void)
{
    rcc_periph_reset_pulse(RST_TIM2);

    timer_set_prescaler(TIM2, PSC);
    timer_direction_up(TIM2);
    timer_disable_preload(TIM2);
    timer_update_on_overflow(TIM2);
    timer_enable_update_event(TIM2);
    timer_continuous_mode(TIM2);

    timer_set_oc_fast_mode(TIM2, TIM_OC1);
    timer_set_oc_value(TIM2, TIM_OC1, 1);

    nvic_enable_irq(NVIC_TIM2_IRQ);
    timer_enable_irq(TIM2, TIM_DIER_UIE);
    timer_enable_irq(TIM2, TIM_DIER_CC1IE);
}

static void gpio_setup(void)
{
    /* Set GPIO (in GPIO port C) to 'output push-pull'. */
    gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_10_MHZ,
                  GPIO_CNF_OUTPUT_PUSHPULL, GPIO13);

    // X - step
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_10_MHZ,
                  GPIO_CNF_OUTPUT_OPENDRAIN, GPIO0);
    // Y - step
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_10_MHZ,
                  GPIO_CNF_OUTPUT_OPENDRAIN, GPIO3);
    // Z - step
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_10_MHZ,
                  GPIO_CNF_OUTPUT_OPENDRAIN, GPIO6);
    // X - dir
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_10_MHZ,
                  GPIO_CNF_OUTPUT_OPENDRAIN, GPIO1);
    // Y - dir
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_10_MHZ,
                  GPIO_CNF_OUTPUT_OPENDRAIN, GPIO4);
    // Z - dir
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_10_MHZ,
                  GPIO_CNF_OUTPUT_OPENDRAIN, GPIO7);

    // Z - stop
    gpio_set_mode(GPIOB, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, GPIO12);
    gpio_set(GPIOB, GPIO12);
    // Y - stop
    gpio_set_mode(GPIOB, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, GPIO13);
    gpio_set(GPIOB, GPIO13);
    // X - stop
    gpio_set_mode(GPIOB, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, GPIO14);
    gpio_set(GPIOB, GPIO14);
    // Z - probe
    gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, GPIO15);
    gpio_set(GPIOA, GPIO15);
}


/******** SHELL ******************/

static void transmit_char(unsigned char data)
{
    USART_DR(USART1) = data;
}

void usart1_isr(void)
{
    /* Check if we were called because of RXNE. */
    if (USART_SR(USART1) & USART_SR_RXNE)
    {
        /* Retrieve the data from the peripheral. */
        unsigned char data = usart_recv(USART1);
        serial_cbs->byte_received(data);
    }

    if (USART_SR(USART1) & USART_SR_TC)
    {
        USART_SR(USART1) &= ~USART_SR_TC;
        serial_cbs->byte_transmitted();
    }
}

static void init_shell(void)
{
    // configure serial_io to use this hardware
    init_serial_shell();

    // configure serial
    serial_cbs->register_byte_transmit(transmit_char);

    // configure shell
    shell_print_init(shell_cbs);
    shell_read_init(shell_cbs);
}

/************* END SHELL ****************/

static volatile int moving;

static void set_dir(int i, int dir)
{
    if (dir) {
        switch (i) {
        case 0:
            gpio_set(GPIOA, GPIO1);
            break;
        case 1:
            gpio_set(GPIOA, GPIO4);
            break;
        case 2:
            gpio_set(GPIOA, GPIO7);
            break;
        }
    } else {
        switch (i) {
        case 0:
            gpio_clear(GPIOA, GPIO1);
            break;
        case 1:
            gpio_clear(GPIOA, GPIO4);
            break;
        case 2:
            gpio_clear(GPIOA, GPIO7);
            break;
        }
    }
}

static void make_step(int i)
{
    switch (i)
    {
    case 0:
        gpio_clear(GPIOA, GPIO0);
        break;
    case 1:
        gpio_clear(GPIOA, GPIO3);
        break;
    case 2:
        gpio_clear(GPIOA, GPIO6);
        break;
    }
}

static void end_step(void)
{
    gpio_set(GPIOA, GPIO0);
    gpio_set(GPIOA, GPIO3);
    gpio_set(GPIOA, GPIO6);
}

static void make_tick(void)
{
    int delay_us = moves_step_tick();
    if (delay_us < 0)
    {
        return;
    }
    int delay = delay_us * FTIMER / 1000000UL;
    if (delay < 3)
        delay = 3;
    timer_set_period(TIM2, delay);
}

void tim2_isr(void)
{
    if (TIM_SR(TIM2) & TIM_SR_UIF) {
        // next step of movement
        // it can set STEP pins active (low)
        TIM_SR(TIM2) &= ~TIM_SR_UIF;
        make_tick();
    }
    else if (TIM_SR(TIM2) & TIM_SR_CC1IF) {
        // set STEP pins not active (high) at the end of STEP
        // ______     _______
        //       |___|
        //
        //           ^
        //           |
        //           here
        TIM_SR(TIM2) &= ~TIM_SR_CC1IF;
        end_step();
    }
}

static void line_started(void)
{
    // PC13 has LED. Enable it
    gpio_clear(GPIOC, GPIO13);

    // Set initial STEP state
    end_step();
    moving = 1;

    // some big enough value, it will be overwritten
    timer_set_period(TIM2, 10000);
    timer_set_counter(TIM2, 0);
    timer_enable_counter(TIM2);
    // first tick
    make_tick();
}

static void line_finished(void)
{
    timer_disable_counter(TIM2);

    // disable LED
    gpio_set(GPIOC, GPIO13);

    // initial STEP state
    end_step();
    moving = 0;
}

static void line_error(void)
{
    // temporary do same things as in finished case
    timer_disable_counter(TIM2);
    gpio_set(GPIOC, GPIO13);
    end_step();
    moving = 0;
}

static cnc_endstops get_stops(void)
{
    cnc_endstops stops = {
        .stop_x  = !(gpio_get(GPIOB, GPIO14) >> 14),
        .stop_y  = !(gpio_get(GPIOB, GPIO13) >> 13),
        .stop_z  = !(gpio_get(GPIOB, GPIO12) >> 12),
        .probe_z = !(gpio_get(GPIOA, GPIO15) >> 15),
    };

    return stops;
}

static void reboot(void)
{
    SCB_AIRCR = (0x5FA<<16)|(1 << 2);
    for (;;)
        ;
}

void config_steppers(steppers_definition *sd)
{
    sd->reboot         = reboot;
    sd->set_dir        = set_dir;
    sd->make_step      = make_step;
    sd->get_endstops   = get_stops;
    sd->line_started   = line_started;
    sd->line_finished  = line_finished;
    sd->line_error     = line_error;
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

void hardware_setup(void)
{
    moving = 0;

    SCB_VTOR = (uint32_t) 0x08000000;

    clock_setup();
    gpio_setup();
    init_shell();
    step_timer_setup();
    usart_setup(SHELL_BAUDRATE);

    gpio_set(GPIOC, GPIO13);
}

