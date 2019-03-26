#pragma once

#include <stdint.h>
#include <moves.h>
#include <arc.h>
#include <stdbool.h>

int empty_slots(void);

void init_planner(steppers_definition pd,
                  void (*arg_send_queued)(int nid),
                  void (*arg_send_started)(int nid),
                  void (*arg_send_completed)(int nid),
                  void (*arg_send_dropped)(int nid));

int planner_line_to(double x[3], double feed, double f0, double f1, int32_t acc, int nid);

int planner_arc_to(double x[3], double d, arc_plane plane, int cw, double feed, double f0, double f1, int32_t acc, int nid);

int planner_function(void (*f)(void), int nid);

void planner_pre_calculate(void);

void enable_break_on_probe(bool en);

extern steppers_definition def;
