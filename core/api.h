#pragma once

// this functions MUST be implemented in firmware

extern int32_t steps_per_unit[3];

// reset firmware
void system_reset(void);

// send answer to user
void print_answer(int res, char *ans, int anslen);

// set direction of steps
void set_dir(int i, int neg);

// send step to axe i
void make_step(int i);

// begin of line
void line_started(void);

// end of line
void line_finished(void);
