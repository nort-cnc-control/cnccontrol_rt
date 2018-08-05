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

typedef struct {
	int32_t x[3];
	int32_t s[3];
	int32_t maxi;
	int32_t feed;
	int32_t feed0;
	int32_t feed1;
	int32_t len;
	int32_t steps;
	int32_t acc_steps;
	int32_t dec_steps;
	int32_t acceleration;
} line_plan;

void init_moves(steppers_definition definition);

int move_line_to(line_plan *plan);

void find_begin(int rx, int ry, int rz);

void set_acceleration(int32_t acc);

void pre_calculate(line_plan *line);


// step of moving
int step_tick(void);

cnc_endstops moves_get_endstops(void);

extern cnc_position position;

