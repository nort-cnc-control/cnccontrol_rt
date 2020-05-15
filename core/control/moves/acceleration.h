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
    _Decimal64 acceleration;
    _Decimal64 feed;
    _Decimal64 target_feed;
    _Decimal64 end_feed;
} acceleration_state;

void acceleration_process(acceleration_state *state, int32_t step_delay);
