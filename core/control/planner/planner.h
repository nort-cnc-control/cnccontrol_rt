#pragma once

#include <stdint.h>
#include <moves.h>

int empty_slots(void);

void init_planner(steppers_definition pd,
                  void (*arg_send_queued)(int nid),
                  void (*arg_send_started)(int nid),
                  void (*arg_send_completed)(int nid));


int planner_line_to(fixed x[3], int32_t feed, int32_t f0, int32_t f1, int32_t acc, int nid);

int planner_function(void (*f)(void), int nid);

void planner_z_probe(int nid);

void planner_find_begin(int rx, int ry, int rz, int nid);

void planner_pre_calculate(void);

extern steppers_definition def;

