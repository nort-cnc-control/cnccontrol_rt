#pragma once

#include <control/control.h>
#include <control/moves/moves_common/steppers.h>

void steppers_setup(void);

void steppers_config(steppers_definition *sd, gpio_definition *gd);

