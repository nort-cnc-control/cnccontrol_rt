#include "platform.h"

#include <shell.h>

#ifdef CONFIG_LIBCORE
#include "steppers.h"
#endif

#ifdef CONFIG_UART
#include "uart.h"
#endif

#ifdef CONFIG_LIBMODBUS
static void modbus_uart_send(const uint8_t *data, size_t len)
{
    uart_send_modbus(data, len);
}
#else
static void (*modbus_uart_send)(const uint8_t *data, size_t len) = NULL;
#endif

void hardware_setup(void)
{
#ifdef CONFIG_UART
    uart_setup();
#endif

#ifdef CONFIG_LIBCORE
    steppers_setup();
#endif

    shell_setup(NULL, modbus_uart_send);
}

void hardware_loop(void)
{
#ifdef CONFIG_BOARD_MEGA2560_CONTROL_UART
    if (uart_message_received)
    {
        uart_message_received = false;
        shell_message_completed();
    }

    if (shell_has_pending_messages())
    {
        ssize_t len;
        const uint8_t* data = shell_pick_message(&len);
        uart_send_control(data, len);
        uart_send_control("\n\r", 2);
        shell_send_completed();
    }
#endif
}

