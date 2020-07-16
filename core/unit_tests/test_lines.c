#include <line.h>
#include <err.h>
#include <assert.h>
#include <stdio.h>

#define ACC 50
#define STEPS_PER_MM 400
#define FEED_BASE 5
#define FEED_MAX 1500

#define SIZE_X 1000
#define SIZE_Y 1000
#define SIZE_Z 1000

static volatile int moving = 0;

static void line_started(void)
{
    printf("Line started\n");
    moving = 1;
}


static void line_finished(void)
{
    printf("Line finished\n");
    moving = 0;
}

static void line_error(void)
{
    printf("Line error\n");
    moving = 0;
}

static int s[3], d[3] = {1, 1, 1};

static cnc_endstops get_stops(void)
{
    cnc_endstops stops = {
        .stop_x = s[0] <= 0,
        .stop_y = s[1] <= 0,
        .stop_z = s[2] <= 0,
        .probe = 0,
    };

    return stops;
}

static void make_step(int i)
{
    s[i] += d[i];
}

static void set_dir(int i, bool dir)
{
    if (dir)
        d[i] = 1;
    else
        d[i] = -1;
}

static void init_lines(void)
{
    static steppers_definition sd = {
        .set_dir        = set_dir,
        .make_step      = make_step,
        .get_endstops   = get_stops,
        .line_started   = line_started,
        .line_finished  = line_finished,
        .line_error     = line_error,
        .steps_per_unit = {
            STEPS_PER_MM,
            STEPS_PER_MM,
            STEPS_PER_MM
        },
        .feed_base = FEED_BASE,
        .feed_max = FEED_MAX,
        .size = {
            SIZE_X,
            SIZE_Y,
            SIZE_Z,
        },
        .acc_default = ACC,
    };

    moves_common_init(&sd);
}

void test_line_x(void)
{
    int i;
    printf("\ntest_line_x\n");
    line_plan plan = {
        .x = 	{
            10*STEPS_PER_MM,
            0,
            0
        },
        .feed = 100,
        .feed0 = 0,
        .feed1 = 0,
        .acceleration = ACC,
    };

    init_lines();
    line_pre_calculate(&plan);

    for (i = 0; i < 3; i++)
        s[i] = 0;
    int res = line_move_to(&plan);
    assert(res == -E_OK);

    assert(moving == 1);
    while (moving)
        line_step_tick();
    for (i = 0; i < 3; i++)
        assert(s[i] == plan.x[i]);
}

void test_line_xyz(void)
{
    int i;
    printf("\ntest_line_xyz\n");
    line_plan plan = {
        .x = 	{
            10*STEPS_PER_MM,
            1*STEPS_PER_MM,
            1*STEPS_PER_MM
        },
        .feed = 100,
        .feed0 = 0,
        .feed1 = 0,
        .acceleration = ACC,
    };

    init_lines();
    line_pre_calculate(&plan);

    for (i = 0; i < 3; i++)
        s[i] = 0;
    int res = line_move_to(&plan);
    assert(res == -E_OK);
    assert(moving == 1);
    while (moving)
        line_step_tick();
    for (i = 0; i < 3; i++)
        assert(s[i] == plan.x[i]);
}

void test_line_empty(void)
{
    int i;
    printf("\ntest_line_next\n");
    line_plan plan = {
        .x = 	{
            0,
            0,
            0
        },
        .feed = 100,
        .feed0 = 0,
        .feed1 = 0,
        .acceleration = ACC,
    };

    init_lines();
    line_pre_calculate(&plan);

    int res = line_move_to(&plan);
    assert(res == -E_NEXT);
}

void test_multiple_lines(void)
{
    int i;
    printf("\ntest_multiple_lines\n");
    line_plan plan1 = {
        .x = 	{
            4000,
            0,
            0
        },
        .feed = 15,
        .feed0 = 0,
        .feed1 = 0,
        .acceleration = 40,
    };

    init_lines();
    line_pre_calculate(&plan1);

    for (i = 0; i < 3; i++)
        s[i] = 0;

    int res = line_move_to(&plan1);
    assert(res == -E_OK);
    assert(moving == 1);
    while (moving)
        line_step_tick();

    printf("pos1 = %i %i %i\n", s[0], s[1], s[2]);
    for (i = 0; i < 3; i++)
    {
        assert(s[i] == plan1.x[i]);
    }

    line_plan plan2 = {
        .x = 	{
            -4000,
            0,
            0
        },
        .feed = 15,
        .feed0 = 0,
        .feed1 = 0,
        .acceleration = 40,
    };

    line_pre_calculate(&plan2);

    res = line_move_to(&plan2);
    assert(res == -E_OK);
    assert(moving == 1);
    while (moving)
        line_step_tick();

    printf("pos2 = %i %i %i\n", s[0], s[1], s[2]);
    for (i = 0; i < 3; i++)
    {
        assert(s[i] == plan1.x[i] + plan2.x[i]);
    }

    line_plan plan3 = {
        .x = 	{
            4000,
            0,
            0
        },
        .feed = 15,
        .feed0 = 0,
        .feed1 = 0,
        .acceleration = 40,
    };

    line_pre_calculate(&plan3);

    res = line_move_to(&plan3);
    assert(res == -E_OK);
    assert(moving == 1);
    while (moving)
        line_step_tick();

    printf("pos3 = %i %i %i\n", s[0], s[1], s[2]);
    for (i = 0; i < 3; i++)
    {
        assert(s[i] == plan1.x[i] + plan2.x[i] + plan3.x[i]);
    }

}

int main(void)
{
/*    test_line_x();
    test_line_xyz();
    test_line_empty();*/
    test_multiple_lines();
    return 0;
}

