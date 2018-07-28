#pragma once

#include <stdint.h>

typedef struct {
	uint8_t en:1;
	uint8_t dir:1;
} step_flags;

typedef struct {
	int32_t pos[3];
	int32_t speed[3];
	step_flags flags[3];
	uint8_t abs_crd : 1;
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
	int32_t steps_per_unit[3];
	int32_t accelerations[3];
	cnc_endstops (*get_endstops)(void);
} steppers_definition;

void init_moves(steppers_definition definition);

void move_line_to(int32_t x[3]);

void find_begin(int rx, int ry, int rz);

void set_speed(int32_t speed);

// step of moving
int step_tick(void);

cnc_endstops moves_get_endstops(void);

extern cnc_position position;
extern int busy;

