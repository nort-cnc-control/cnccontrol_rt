#pragma once

#include <stdint.h>

typedef enum {
    STATE_STOP = 0,
    STATE_ACC,
    STATE_GO,
    STATE_DEC,
    STATE_STOP_COMPLETION,
} acceleration_type;

typedef struct {
    acceleration_type type;

    float start_t;
    float end_t;
    float acc_t;
    float dec_t;
    float current_t;

    double acceleration;
    double feed;
    double target_feed;
    double end_feed;
} acceleration_state;

void acceleration_process(acceleration_state *state, double step_delay, double current_t);

double accelerate(double feed, double acc, double delay);
double acceleration(double feed0,
                    double feed1,
                    double acc,
                    double len,
                    double begin,
                    double end);


