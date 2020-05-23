#pragma once

#include <stdbool.h>

typedef struct
{
	void (*set_gpio)(int id, int state);
} gpio_definition;

typedef struct
{
    bool on;
    int id;
} tool_plan;

void tools_init(gpio_definition *definition);
int tool_action(tool_plan *plan);

