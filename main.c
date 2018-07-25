#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/scb.h>
#include <shell/shell.h>
#include <control/control.h>
#include <control/moves.h>

#define FULL_STEPS 200
#define MICRO_STEP 4

#define STEPS_PER_ROUND ((FULL_STEPS) * (MICRO_STEP))
#define MM_PER_ROUND 4.0

#define STEPS_PER_MM ((STEPS_PER_ROUND) / (MM_PER_ROUND))

static void clock_setup(void)
{
        rcc_clock_setup_in_hse_8mhz_out_72mhz();

        /* Enable GPIOA clock. */
        rcc_periph_clock_enable(RCC_GPIOA);
        rcc_periph_clock_enable(RCC_GPIOC);

        /* Enable clocks for GPIO port A (for GPIO_USART1_TX) and USART1. */
        rcc_periph_clock_enable(RCC_USART1);
}

static void usart_setup(int baudrate)
{
	nvic_enable_irq(NVIC_USART1_IRQ);

	/* Setup GPIO pin GPIO_USART1_RE_RX on GPIO port A for receive. */
	gpio_set_mode(GPIOA, GPIO_MODE_INPUT,
                      GPIO_CNF_INPUT_FLOAT, GPIO_USART1_RX);

        /* Setup GPIO pin GPIO_USART1_TX. */
        gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
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

void usart1_isr(void)
{
	/* Check if we were called because of RXNE. */
	if (USART_SR(USART1) & USART_SR_RXNE) {
		/* Retrieve the data from the peripheral. */
		char data = usart_recv(USART1);
		shell_char_received(data);
	}

	if (USART_SR(USART1) & USART_SR_TC) {
		shell_char_transmitted();
		USART_SR(USART1) &= ~USART_SR_TC;
	}
}


static void gpio_setup(void)
{
        /* Set GPIO (in GPIO port C) to 'output push-pull'. */
        gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_2_MHZ,
                      GPIO_CNF_OUTPUT_PUSHPULL, GPIO13);

	// X - step
        gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_2_MHZ,
                      GPIO_CNF_OUTPUT_OPENDRAIN, GPIO0);
	// Y - step
        gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_2_MHZ,
                      GPIO_CNF_OUTPUT_OPENDRAIN, GPIO3);
	// Z - step
        gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_2_MHZ,
                      GPIO_CNF_OUTPUT_OPENDRAIN, GPIO6);
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

static volatile int step_x, step_y, step_z;

static void set_dir(int i, int dir)
{

}

static void make_step(int i)
{
	switch (i)
	{
	case 0:
		step_x = 1;
		break;
	case 1:
		step_y = 1;
		break;
	case 2:
		step_z = 1;
		break;
	}
}

static void line_started(void)
{
	gpio_set(GPIOA, GPIO0);
	gpio_set(GPIOA, GPIO3);
	gpio_set(GPIOA, GPIO6);
	moving = 1;
}

static void line_finished(void)
{
	gpio_set(GPIOA, GPIO0);
	gpio_set(GPIOA, GPIO3);
	gpio_set(GPIOA, GPIO6);
	moving = 0;
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
			STEPS_PER_MM,
		},
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
	usart_setup(9600);

        while (1) {
		// stepping loop
		if (step_x)
			gpio_clear(GPIOA, GPIO0);
		if (step_y)
			gpio_clear(GPIOA, GPIO3);
		if (step_z)
			gpio_clear(GPIOA, GPIO6);
		step_x = step_y = step_z = 0;
		for (c = 0; c < 100; c++)
			__asm__("nop");
		gpio_set(GPIOA, GPIO0);
	}

        return 0;
}

