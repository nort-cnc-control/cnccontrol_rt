#include <avr/io.h>

#include <stdio.h>
#include <string.h>

#include "config.h"

#include <stdbool.h>
#include <shell.h>

#define UDR(x)    CONCAT2(UDR, x)
#define UCSRA(x)  CONCAT3(UCSR, x, A)
#define UCSRB(x)  CONCAT3(UCSR, x, B)
#define UCSRC(x)  CONCAT3(UCSR, x, C)

#define UBRRL(x)  CONCAT3(UBRR, x, L)
#define UBRRH(x)  CONCAT3(UBRR, x, H)

#define TXC(x)    CONCAT2(TXC, x)
#define RXEN(x)   CONCAT2(RXEN, x)
#define TXEN(x)   CONCAT2(TXEN, x)
#define UCSZ2(x)  CONCAT2(UCSZ2, x)
#define UCSZ1(x)  CONCAT2(UCSZ1, x)
#define UCSZ0(x)  CONCAT2(UCSZ0, x)

#define RXCIE(x)  CONCAT2(RXCIE, x)
#define TXCIE(x)  CONCAT2(TXCIE, x)

#define USART_RX_vect(x) CONCAT3(USART, x, _RX_vect)

#ifdef CONFIG_LIBMODBUS
void uart_send_modbus(const uint8_t *data, size_t len)
{
    /* RTS */
    PORT(MODBUS_UART_RTS_PORT) &= ~(1 << MODBUS_UART_RTS_PIN);

    while (len > 0)
    {
        uint16_t d = *(data++);
        while (UCSRA(MODBUS_UART_PORT) & TXC(MODBUS_UART_PORT) == 0)
            ;
        UDR(MODBUS_UART_PORT) = d;
        len--;
    }

    PORT(MODBUS_UART_RTS_PORT) |= 1 << MODBUS_UART_RTS_PIN;
}
#endif

#ifdef CONFIG_BOARD_MEGA2560_CONTROL_UART
bool uart_message_received = false;

void uart_send_control(const uint8_t *data, size_t len)
{
    while (len > 0)
    {
        uint16_t d = *(data++);
        while (UCSRA(CONTROL_UART_PORT) & TXC(CONTROL_UART_PORT) == 0)
            ;
        UDR(CONTROL_UART_PORT) = d;
        len--;
    }
}

ISR(USART_RX_vect(CONTROL_UART_PORT))
{
    uint8_t c = UDR(CONTROL_UART_PORT);
    if (c == '\n' || c == '\r')
    {
        uart_message_received = true;
    }
    else
    {
        shell_data_received(&c, 1);
    }
}

#endif


void uart_setup(void)
{
    uint16_t ubrr;
#ifdef CONFIG_LIBMODBUS
    UCSRA(MODBUS_UART_PORT) = 0;
    UCSRB(MODBUS_UART_PORT) = (1 << TXEN(MODBUS_UART_PORT));
    UCSRC(MODBUS_UART_PORT) = (1 << UCSZ1(MODBUS_UART_PORT)) | (1 << UCSZ0(MODBUS_UART_PORT));

    ubrr = F_CPU / 16 / MODBUS_UART_BAUDRATE - 1;
    UBRRL(MODBUS_UART_PORT) = ubrr & 0xFF;
    UBRRH(MODBUS_UART_PORT) = (ubrr >> 8) & 0xFF;
#endif

#ifdef CONFIG_BOARD_MEGA2560_CONTROL_UART
    UCSRA(CONTROL_UART_PORT) = 0;
    UCSRB(CONTROL_UART_PORT) = (1 << TXEN(CONTROL_UART_PORT)) | (1 << RXEN(CONTROL_UART_PORT)) | (1 << RXCIE(CONTROL_UART_PORT));
    UCSRC(CONTROL_UART_PORT) = (1 << UCSZ1(CONTROL_UART_PORT)) | (1 << UCSZ0(CONTROL_UART_PORT));  

    ubrr = F_CPU / 16 / CONTROL_UART_BAUDRATE - 1;
    UBRRL(CONTROL_UART_PORT) = ubrr & 0xFF;
    UBRRH(CONTROL_UART_PORT) = (ubrr >> 8) & 0xFF;
#endif
}

