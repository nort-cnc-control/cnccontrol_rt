#pragma once

#include <stdint.h>

typedef struct {
	uint8_t stop_x:1;
	uint8_t stop_y:1;
	uint8_t stop_z:1;
	uint8_t probe_z:1;
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
	int32_t es_travel;
	int32_t es_precise;
	int32_t probe_travel;
	int32_t probe_precise;
	int32_t size[3];
	int32_t acc_default;
	int32_t feed_default;
} steppers_definition;

