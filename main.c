#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/scb.h>
#include <shell/shell.h>


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
}


static void transmit_char(char data)
{
	USART_DR(USART1) = data;
}

static void shell_line_received(const char *cmd)
{
	shell_send_string("ok\n\r");
}

static void init_shell(void)
{
	shell_cbs cbs = {
		.line_received = shell_line_received,
		.transmit_char = transmit_char,
	};
	shell_init(cbs);
}

int main(void)
{
	
        int i, j = 0, c = 0;

	SCB_VTOR = (uint32_t) 0x08000000;	

        clock_setup();
        gpio_setup();
	init_shell();
	usart_setup(9600);

	shell_send_string("Hello\r\n");

        while (1) {
        }

        return 0;
}

