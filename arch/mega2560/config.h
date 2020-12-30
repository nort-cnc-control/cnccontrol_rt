#pragma once

#define SHELL_BAUDRATE 38400
#define LOCK_ON_CRC_ERROR 0

/* X configure */
#define X_STEP_PORT F
#define X_STEP_PIN  0

#define X_DIR_PORT  F
#define X_DIR_PIN   1

#define X_EN_PORT   D
#define X_EN_PIN    7

#define X_MIN_PORT  E
#define X_MIN_PIN   5

#define X_MAX_PORT  D
#define X_MAX_PIN   1

/* Y configure */
#define Y_STEP_PORT F
#define Y_STEP_PIN  6

#define Y_DIR_PORT  F
#define Y_DIR_PIN   7

#define Y_EN_PORT   F
#define Y_EN_PIN    2

#define Y_MIN_PORT  J
#define Y_MIN_PIN   1

#define Y_MAX_PORT  J
#define Y_MAX_PIN   0

/* Z configure */
#define Z_STEP_PORT L
#define Z_STEP_PIN  3

#define Z_DIR_PORT  L
#define Z_DIR_PIN   1

#define Z_EN_PORT   K
#define Z_EN_PIN    0

#define Z_MIN_PORT  D
#define Z_MIN_PIN   3

#define Z_MAX_PORT  D
#define Z_MAX_PIN   2

/* Probe configure */
#define PROBE_PORT  K
#define PROBE_PIN   1

/* GPIO tool 0 - SER1*/
#define TOOL0_PORT  B
#define TOOL0_PIN   5

/* GPIO tool 1 - PS ON */
#define TOOL1_PORT  B
#define TOOL1_PIN   6

/* Indicator LED */
#define LED_PORT    B
#define LED_PIN     7

/* Modbus UART */
#define MODBUS_UART_PORT     2
#define MODBUS_UART_BAUDRATE 9600UL
#define MODBUS_UART_RTS_PORT A
#define MODBUS_UART_RTS_PIN  1

/* Control UART */
#define CONTROL_UART_PORT     0
#define CONTROL_UART_BAUDRATE 38400UL

#define CONCAT2(a, b) (a##b)
#define CONCAT3(a, b, c) (a##b##c)

#define DDR(x)  CONCAT2(DDR, x)
#define PORT(x) CONCAT2(PORT, x)
#define PIN(x)  CONCAT2(PIN, x)

#define gpio_get(port, pin) ((PIN(port) & (1 << (pin))) >> (pin))
#define gpio_set(port, pin) (PORT(port) |= (1 << (pin)))
#define gpio_clear(port, pin) (PORT(port) &= ~(1 << (pin)))


