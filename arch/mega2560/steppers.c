#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

#include <stdio.h>
#include <string.h>

#include "config.h"
#include "steppers.h"

#define PSC 64
#define FTIMER (F_CPU / PSC)
#define TIMEOUT_TIMER_STEP 1000UL

static bool going = false;

static void steppers_timer_setup(void)
{
    TCCR1A = 0 << WGM11 | 0 << WGM10;
    TCCR1B = 0 << WGM13 | 1 << WGM12;
    TCCR1C = 0;
    TIMSK1 |= 1 << OCIE1B | 1 << OCIE1A;
    OCR1B = 3;
}

void steppers_setup(void)
{
    steppers_timer_setup();

    DDR(X_STEP_PORT) |= 1 << X_STEP_PIN;
    DDR(X_DIR_PORT)  |= 1 << X_DIR_PIN;
    DDR(X_EN_PORT)   |= 1 << X_EN_PIN; // X

    DDR(Y_STEP_PORT) |= 1 << Y_STEP_PIN;
    DDR(Y_DIR_PORT)  |= 1 << Y_DIR_PIN;
    DDR(Y_EN_PORT)   |= 1 << Y_EN_PIN; // Y

    DDR(Z_STEP_PORT) |= 1 << Z_STEP_PIN;
    DDR(Z_DIR_PORT)  |= 1 << Z_DIR_PIN;
    DDR(Z_EN_PORT)   |= 1 << Z_EN_PIN; // Z
 
    // X - stop
    DDR(X_MIN_PORT)  &= ~(1 << X_MIN_PIN);
    PORT(X_MIN_PORT) |= 1 << X_MIN_PIN;
    DDR(X_MAX_PORT)  &= ~(1 << X_MAX_PIN);
    PORT(X_MAX_PORT) |= 1 << X_MAX_PIN;

    // Y - stop
    DDR(Y_MIN_PORT)  &= ~(1 << Y_MIN_PIN);
    PORT(Y_MIN_PORT) |= 1 << Y_MIN_PIN;
    DDR(Y_MAX_PORT)  &= ~(1 << Y_MAX_PIN);
    PORT(Y_MAX_PORT) |= 1 << Y_MAX_PIN;

    // Y - stop
    DDR(Z_MIN_PORT)  &= ~(1 << Z_MIN_PIN);
    PORT(Z_MIN_PORT) |= 1 << Z_MIN_PIN;
    DDR(Z_MAX_PORT)  &= ~(1 << Z_MAX_PIN);
    PORT(Z_MAX_PORT) |= 1 << Z_MAX_PIN;

    // Probe
    DDR(PROBE_PORT)  &= ~(1 << PROBE_PIN);
    PORT(PROBE_PORT) |= 1 << PROBE_PIN;

    // GPIO Tool 0
    DDR(TOOL0_PORT)  |= 1 << TOOL0_PIN; // SER1

    // Indication LED
    DDR(LED_PORT)    |= 1 << LED_PIN;
    PORT(LED_PORT)   &= ~(1 << LED_PIN);
}

static volatile int moving;

static void set_dir(int i, bool dir)
{
    if (dir) {
        switch (i) {
        case 0: // X
            gpio_set(X_DIR_PORT, X_DIR_PIN);
            break;
        case 1: // Y
            gpio_set(Y_DIR_PORT, Y_DIR_PIN);
            break;
        case 2: // Z
            gpio_set(Z_DIR_PORT, Z_DIR_PIN);
            break;
        }
    } else {
        switch (i) {
        case 0: // X
            gpio_clear(X_DIR_PORT, X_DIR_PIN);
            break;
        case 1: // Y
            gpio_clear(Y_DIR_PORT, Y_DIR_PIN);
            break;
        case 2: // Z
            gpio_clear(Z_DIR_PORT, Z_DIR_PIN);
            break;
        }
    }
}

static void make_step(int i)
{
    switch (i)
    {
    case 0: // X
        gpio_set(X_STEP_PORT, X_STEP_PIN);
        break;
    case 1: // Y
        gpio_set(Y_STEP_PORT, Y_STEP_PIN);
        break;
    case 2: // Z
        gpio_set(Z_STEP_PORT, Z_STEP_PIN);
        break;
    }
}

static void end_step(void)
{
    gpio_clear(X_STEP_PORT, X_STEP_PIN); // X
    gpio_clear(Y_STEP_PORT, Y_STEP_PIN); // Y
    gpio_clear(Z_STEP_PORT, Z_STEP_PIN); // Z
}

static void make_tick(void)
{
    int delay_us = moves_step_tick();
    if (delay_us < 0)
    {
        return;
    }
    int delay = delay_us * FTIMER / 1000000UL;
    if (delay < 5)
        delay = 5;
    OCR1A = delay;
    TCNT1 = 0;
}

ISR(TIMER1_COMPA_vect)
{
    if (!going)
    {
        TCCR1B &= ~(1 << CS12 | 1 << CS11 | 1 << CS10);
        return;
    }
    
    // next step of movement
    // it can set STEP pins active (low)
    make_tick();
}

ISR(TIMER1_COMPB_vect)
{
    // set STEP pins not active (low) at the end of STEP
    //        ___
    // ______|   |_______
    //
    //           ^
    //           |
    //           here
    end_step();
}

static void line_started(void)
{
    // PB7 has LED. Enable it
    gpio_set(LED_PORT, LED_PIN);

    // Set initial STEP state
    end_step();
    moving = 1;

    // first tick
    going = true;
    make_tick();
    if (going)
    {
        TCNT1 = 0;
        TCCR1B |= (0 << CS12 | 1 << CS11 | 1 << CS10);
    }
}

static void line_finished(void)
{
    TCCR1B &= ~(1 << CS12 | 1 << CS11 | 1 << CS10);
    going = false;
    
    // disable LED
    gpio_clear(LED_PORT, LED_PIN);

    // initial STEP state
    end_step();
    moving = 0;
}

static void line_error(void)
{
    // temporary do same things as in finished case
    TCCR1B &= ~(1 << CS12 | 1 << CS11 | 1 << CS10);
    going = false;
    
    gpio_set(LED_PORT, LED_PIN);
    end_step();
    moving = 0;
}

static void set_gpio(int id, int on)
{
    switch (id)
    {
    case 0:
        if (on)
            gpio_clear(TOOL0_PORT, TOOL0_PIN);
        else
            gpio_set(TOOL0_PORT, TOOL0_PIN);
        break;
    }
}

static cnc_endstops get_stops(void)
{
    cnc_endstops stops = {
        .stop_x  = !(gpio_get(X_MIN_PORT, X_MIN_PIN)),
        .stop_y  = !(gpio_get(Y_MIN_PORT, Y_MIN_PIN)),
        .stop_z  = !(gpio_get(Z_MIN_PORT, Z_MIN_PIN)),
        .probe   = !(gpio_get(PROBE_PORT, PROBE_PIN)),
    };

    return stops;
}

static void reboot(void)
{
    cli(); //irq's off
    wdt_enable(WDTO_15MS); //wd on,15ms
    while(1); //loop
}

void steppers_config(steppers_definition *sd, gpio_definition *gd)
{
    sd->reboot         = reboot;
    sd->set_dir        = set_dir;
    sd->make_step      = make_step;
    sd->get_endstops   = get_stops;
    sd->line_started   = line_started;
    sd->line_finished  = line_finished;
    sd->line_error     = line_error;
    gd->set_gpio       = set_gpio;
}

