#pragma once

#include <stdint.h>
#include <moves.h>
#include <tools.h>
#include <arc.h>
#include <stdbool.h>

int empty_slots(void);

void init_planner(steppers_definition pd,
		  gpio_definition gd,
                  void (*arg_send_queued)(int nid),
                  void (*arg_send_started)(int nid),
                  void (*arg_send_completed)(int nid),
                  void (*arg_send_dropped)(int nid),
		  void (*arg_send_failed)(int nid));

int planner_line_to(_Decimal64 x[3], _Decimal64 feed, _Decimal64 f0, _Decimal64 f1, int32_t acc, int nid);

int planner_arc_to(_Decimal64 x[3], _Decimal64 d, arc_plane plane, int cw, _Decimal64 feed, _Decimal64 f0, _Decimal64 f1, int32_t acc, int nid);

int planner_tool(int id, bool on, int nid);

void planner_pre_calculate(void);

void enable_break_on_probe(bool en);

void planner_unlock(void);

void planner_lock(void);

int planner_is_locked(void);

void planner_fail_on_endstops(bool fail);

extern steppers_definition def;
