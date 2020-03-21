#include <err.h>
#include <tools.h>

static gpio_definition def;
void tools_init(gpio_definition definition)
{
    def = definition;
}

int tool_action(tool_plan *plan)
{
	def.set_gpio(plan->id, plan->on);
	return -E_NEXT;
}

