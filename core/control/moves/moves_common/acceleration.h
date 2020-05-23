#pragma once

#include <stdint.h>

typedef enum {
    STATE_STOP = 0,
    STATE_ACC,
    STATE_GO,
    STATE_DEC,
} acceleration_type;

typedef struct {
    acceleration_type type;
    int total_steps;
    int acc_steps;
    int dec_steps;
    int step;
    double acceleration;
    double feed;
    double target_feed;
    double end_feed;
} acceleration_state;

void acceleration_process(acceleration_state *state, double step_delay);

