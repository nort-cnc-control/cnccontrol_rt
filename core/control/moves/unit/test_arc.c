#include <arc.h>
#include <stdio.h>

int32_t pos[3];
bool dirs[3];

void set_dir(int i, bool dir)
{
	dirs[i] = dir;
}

void make_step(int i)
{
	if (!dirs[i])
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

	arc_plan plan = {
                .cw = false,
		.x = {0, 4, 0},
                .a = 10.0,
                .b = 10.0,
                .plane = XY,
		.feed = 100,
		.feed0 = 100,
		.feed1 = 100,
		.acceleration = 40,
		.len = -1, // must be negative at init
	};

	pos[0] = pos[1] = pos[2] = 0;
	arc_move_to(&plan);
	int delay = -1;
	do
	{
		delay = arc_step_tick();
		printf("%i %i %i, %i\n", pos[0], pos[1], pos[2], delay);
	} while (delay >= 0);
}

void test_2(void)
{
	steppers_definition def = {
		.set_dir = set_dir,
		.make_step = make_step,
		.steps_per_unit = {1, 1, 1},
	};

	moves_common_init(&def);	

	arc_plan plan = {
                .cw = false,
		.x = {0, 16, 0},
                .a = 10.0,
                .b = 10.0,
                .plane = XY,
		.feed = 100,
		.feed0 = 100,
		.feed1 = 100,
		.acceleration = 40,
		.len = -1, // must be negative at init
	};

	pos[0] = pos[1] = pos[2] = 0;
	printf("%i %i %i\n", pos[0], pos[1], pos[2]);
	arc_move_to(&plan);
	int delay = -1;
	do
	{
		delay = arc_step_tick();
		printf("%i %i %i, %i\n", pos[0], pos[1], pos[2], delay);
	} while (delay >= 0);
}

void test_3(void)
{
	steppers_definition def = {
		.set_dir = set_dir,
		.make_step = make_step,
		.steps_per_unit = {1, 1, 1},
	};

	moves_common_init(&def);	

	arc_plan plan = {
                .cw = false,
		.x = {0, 20, 0},
                .a = 10.0,
                .b = 10.0,
                .plane = XY,
		.feed = 100,
		.feed0 = 100,
		.feed1 = 100,
		.acceleration = 40,
		.len = -1, // must be negative at init
	};

	pos[0] = pos[1] = pos[2] = 0;
	printf("%i %i %i\n", pos[0], pos[1], pos[2]);
	arc_move_to(&plan);
	int delay = -1;
	do
	{
		delay = arc_step_tick();
		printf("%i %i %i, %i\n", pos[0], pos[1], pos[2], delay);
	} while (delay >= 0);
}

void test_4(void)
{
        steppers_definition def = {
                .set_dir = set_dir,
                .make_step = make_step,
                .steps_per_unit = {1, 1, 1},
        };

        moves_common_init(&def);

        arc_plan plan = {
                .cw = false,
                .x = {40000, 0, 0},
                .a = 20000.0,
                .b = 20000.0,
                .plane = XY,
                .feed = 100,
                .feed0 = 100,
                .feed1 = 100,
                .acceleration = 40,
                .len = -1, // must be negative at init
        };

        pos[0] = pos[1] = pos[2] = 0;
        printf("%i %i %i\n", pos[0], pos[1], pos[2]);
        arc_move_to(&plan);
        int delay = -1;
        do
        {
                delay = arc_step_tick();
                printf("%i %i %i, %i\n", pos[0], pos[1], pos[2], delay);
        } while (delay >= 0);
}


int main(void)
{
	test_4();
	return 0;
}


