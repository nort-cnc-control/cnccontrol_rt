#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/scb.h>
#include <libopencm3/stm32/timer.h>

#include <shell/shell.h>
#include <control/control.h>
#include <control/moves.h>

#define FULL_STEPS 200
#define MICRO_STEP 8

#define STEPS_PER_ROUND ((FULL_STEPS) * (MICRO_STEP))
#define MM_PER_ROUND 4.0

#define STEPS_PER_MM ((STEPS_PER_ROUND) / (MM_PER_ROUND))

#define FCPU 72000000UL
#define FTIMER 100000UL
#define PSC ((FCPU) / (FTIMER) - 1)

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

void step_timer_setup(void)
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


void usart1_isr(void)
{
	/* Check if we were called because of RXNE. */
	if (USART_SR(USART1) & USART_SR_RXNE) {
		/* Retrieve the data from the peripheral. */
		char data = usart_recv(USART1);
		shell_char_received(data);
	}

	if (USART_SR(USART1) & USART_SR_TC) {
		USART_SR(USART1) &= ~USART_SR_TC;
		shell_char_transmitted();
	}
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

	// X - stop
	gpio_set_mode(GPIOB, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, GPIO12);
	// Y - stop
	gpio_set_mode(GPIOB, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, GPIO13);
	// Z - stop
	gpio_set_mode(GPIOB, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, GPIO14);
}


static void transmit_char(char data)
{
	USART_DR(USART1) = data;
}

static void shell_line_received(const char *cmd)
{
	execute_g_command(cmd);
}

static void init_shell(void)
{
	shell_cbs cbs = {
		.line_received = shell_line_received,
		.transmit_char = transmit_char,
	};
	shell_init(cbs);
	shell_echo_enable(1);
}

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

void tim2_isr(void)
{
	if (TIM_SR(TIM2) & TIM_SR_UIF) {
		TIM_SR(TIM2) &= ~TIM_SR_UIF;

		int delay_us = step_tick();
		int delay = delay_us * FTIMER / 1000000UL;

		if (delay < 3)
			delay = 3;
		timer_set_period(TIM2, delay);
	}
	else if (TIM_SR(TIM2) & TIM_SR_CC1IF) {
		TIM_SR(TIM2) &= ~TIM_SR_CC1IF;
		end_step();
	}
}

static void line_started(void)
{
	gpio_clear(GPIOC, GPIO13);
	
	gpio_set(GPIOA, GPIO0);
	gpio_set(GPIOA, GPIO3);
	gpio_set(GPIOA, GPIO6);
	moving = 1;

	timer_set_period(TIM2, 1000);
	timer_set_counter(TIM2, 0);	
	timer_enable_counter(TIM2);
}

static void line_finished(void)
{
	timer_disable_counter(TIM2);
	gpio_set(GPIOC, GPIO13);
		
	gpio_set(GPIOA, GPIO0);
	gpio_set(GPIOA, GPIO3);
	gpio_set(GPIOA, GPIO6);
	moving = 0;
}

static cnc_endstops get_stops(void)
{
	cnc_endstops stops = {
		.stop_x = !(gpio_get(GPIOB, GPIO12) >> 12),
		.stop_y = !(gpio_get(GPIOB, GPIO13) >> 13),
		.stop_z = !(gpio_get(GPIOB, GPIO14) >> 14)
	};

	return stops;
}

static void init_steppers(void)
{
	steppers_definition sd = {
		.set_dir        = set_dir,
		.make_step      = make_step,
		.line_started   = line_started,
		.line_finished  = line_finished,
		.steps_per_unit = {
			STEPS_PER_MM,
			STEPS_PER_MM,
			STEPS_PER_MM
		},
		.accelerations = {
			0,
			0,
			0
		},
		.get_endstops   = get_stops,
	};

	line_finished();
	init_moves(sd);
}

int main(void)
{
	moving = 0;
        int i, j = 0, c = 0;

	SCB_VTOR = (uint32_t) 0x08000000;	

        clock_setup();
        gpio_setup();
	init_shell();
	init_steppers();
	step_timer_setup();
	usart_setup(9600);
	
	gpio_set(GPIOC, GPIO13);

        while (1) {
	}

        return 0;
}

