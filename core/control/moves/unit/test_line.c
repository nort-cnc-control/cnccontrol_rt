#include <line.h>
#include <stdio.h>

int32_t pos[3];
bool dirs[3];

void set_dir(int i, bool dir)
{
	dirs[i] = dir;
}

void make_step(int i)
{
	if (dirs[i])
		pos[i]++;
	else
		pos[i]--;
}

void test_1(void)
{
	steppers_definition def = {
		.set_dir = set_dir,
		.make_step = make_step,
		.steps_per_unit = {1, 1, 1},
	};

	moves_common_init(&def);	

	line_plan plan = {
		.x = {100, 10, 0},
		.feed = 100,
		.feed0 = 100,
		.feed1 = 100,
		.acceleration = 40,
		.len = -1, // must be negative at init
	};

	pos[0] = pos[1] = pos[2] = 0;
	line_move_to(&plan);
	int delay = -1;
	do
	{
		delay = line_step_tick();
		printf("%i %i %i, %i\n", pos[0], pos[1], pos[2], delay);
	} while (delay > 0);
}

void test_2(void)
{
	steppers_definition def = {
		.set_dir = set_dir,
		.make_step = make_step,
		.steps_per_unit = {1, 1, 1},
		.feed_base = 0.01,
	};

	moves_common_init(&def);	

	line_plan plan = {
		.x = {100, 10, 0},
		.feed = 20,
		.feed0 = 0,
		.feed1 = 0,
		.acceleration = 40,
		.len = -1, // must be negative at init
	};

	pos[0] = pos[1] = pos[2] = 0;
	line_move_to(&plan);
	int delay = -1;
	do
	{
		delay = line_step_tick();
		printf("%i %i %i, %i\n", pos[0], pos[1], pos[2], delay);
	} while (delay > 0);
}

void test_3(void)
{
	steppers_definition def = {
		.set_dir = set_dir,
		.make_step = make_step,
		.steps_per_unit = {400, 400, 400},
		.feed_base = 0.01,
	};

	moves_common_init(&def);	

	line_plan plan = {
		.x = {100*400, 0, 0},
		.feed = 20,
		.feed0 = 0,
		.feed1 = 0,
		.acceleration = 40,
		.len = -1, // must be negative at init
	};

	pos[0] = pos[1] = pos[2] = 0;
	line_move_to(&plan);
	int delay = -1;
	do
	{
		delay = line_step_tick();
		printf("%i %i %i, %i\n", pos[0], pos[1], pos[2], delay);
	} while (delay > 0);
}

void test_4(void)
{
	steppers_definition def = {
		.set_dir = set_dir,
		.make_step = make_step,
		.steps_per_unit = {400, 400, 400},
		.feed_base = 0.01,
	};

	moves_common_init(&def);	

	line_plan plan = {
		.x = {100*400, 0, 0},
		.feed = 100./60,
		.feed0 = 100./60,
		.feed1 = 100./60,
		.acceleration = 40.0,
		.len = -1, // must be negative at init
	};

	pos[0] = pos[1] = pos[2] = 0;
	line_move_to(&plan);
	int delay = -1;
	int time = 0;
	do
	{
		delay = line_step_tick();
                if (delay > 0)
	                time += delay;
		printf("%i %i %i, %i\n", pos[0], pos[1], pos[2], delay);
	} while (delay > 0);

	printf("%i\n", time);
}


int main(void)
{
	test_4();
	return 0;
}


