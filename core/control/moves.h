#pragma once

#include <stdint.h>

#include "line.h"

typedef struct {
	uint8_t en:1;
	uint8_t dir:1;
} step_flags;

typedef struct {
	int32_t pos[3];
	int32_t speed[3];
	step_flags flags[3];
} cnc_position;

typedef struct {
	uint8_t stop_x:1;
	uint8_t stop_y:1;
	uint8_t stop_z:1;
} cnc_endstops;

typedef struct 
{
	void (*set_dir)(int i, int dir);
	void (*make_step)(int i);
	void (*line_started)(void);
	void (*line_finished)(void);
	void (*line_error)(void);
	cnc_endstops (*get_endstops)(void);
	int32_t steps_per_unit[3];
	int32_t feed_base;
	int32_t feed_max;
	int32_t size[3];
	int32_t acc_default;
	int32_t feed_default;
} steppers_definition;

void moves_init(steppers_definition definition);

void moves_find_begin(int rx, int ry, int rz);

void moves_set_acceleration(int32_t acc);

cnc_endstops moves_get_endstops(void);

int moves_step_tick(void);

extern cnc_position position;
extern cnc_endstops endstops;
extern steppers_definition def;
