#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint8_t stop_x:1;
    uint8_t stop_y:1;
    uint8_t stop_z:1;
    uint8_t probe:1;
} cnc_endstops;

typedef struct
{
    void (*reboot)(void);
    void (*set_dir)(int i, bool dir);
    void (*make_step)(int i);
    void (*enable_step)(bool en);
    void (*line_started)(void);
    void (*line_finished)(void);
    void (*line_error)(void);
    void (*endstops_touched)(void);
    cnc_endstops (*get_endstops)(void);
    double steps_per_unit[3]; // steps / mm
    double feed_base;        // mm / sec
    double feed_max;         // mm / sec
    double acc_default;      // mm / sec^2
    double feed_default;     // mm / sec
    bool   configured;
} steppers_definition;


