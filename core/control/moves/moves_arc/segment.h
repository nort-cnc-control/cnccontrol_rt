#pragma once

#include <stdint.h>

typedef struct {
    int segment;
    double a2;
    double b2;
    int32_t position_x;
    int32_t position_y;
    int dx;
    int32_t x1;
    int32_t y1;
    int32_t x2;
    int32_t y2;
} arc_segment;

