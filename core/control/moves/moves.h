#pragma once

#include <stdint.h>

#include <control/moves/moves_common/steppers.h>
#include <control/moves/moves_line/line.h>
#include <control/moves/moves_arc/arc.h>

void moves_init(const steppers_definition *definition);
void moves_reset(void);
void moves_break(void);

int moves_line_to(line_plan *plan);
int moves_arc_to(arc_plan *plan);

int32_t moves_step_tick(void);

cnc_endstops moves_get_endstops(void);

