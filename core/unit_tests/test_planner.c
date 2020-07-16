#include <line.h>
#include <err.h>
#include <assert.h>
#include <stdio.h>
#include <planner.h>

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

static void send_queued(int nid)
{
    printf("%i queued\n", nid);
}

static void send_started(int nid)
{
    printf("%i started\n", nid);
}

static void send_completed(int nid)
{
    printf("%i completed\n", nid);
}

static void send_completed_with_pos(int nid, const int *pos)
{
    printf("%i completed. X=%i Y=%i Z=%i\n", nid, pos[0], pos[1], pos[2]);
}

static void send_dropped(int nid)
{
    printf("%i dropped\n", nid);
}

static void send_failed(int nid)
{
    printf("%i failed\n", nid);
}

static void init(void)
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

    static gpio_definition gd = {
    };

    init_planner(&sd, &gd, send_queued, send_started, send_completed, send_completed_with_pos, send_dropped, send_failed);
}

void test_line(void)
{
    int i;
    printf("\ntest_line\n");

    s[0] = s[1] = s[2] = 0;

    init();
    int32_t x[3] = {4000, 0, 0};
    planner_unlock();
    planner_line_to(x, 15, 0, 0, 40, 1);

    assert(moving == 1);

    while(moving)
    {
        moves_step_tick();
    }

    assert(s[0] == x[0]);
    assert(s[1] == x[1]);
    assert(s[2] == x[2]);
}

void test_multiple_lines(void)
{
    int i;
    printf("\ntest_multiple_lines\n");
    
    s[0] = s[1] = s[2] = 0;

    init();
    int32_t x1[3] = {4000, 0, 0};
    int32_t x2[3] = {-4000, 0, 0};
    int32_t x3[3] = {4000, 0, 0};
    planner_unlock();
    planner_line_to(x1, 15, 0, 0, 40, 1);
    planner_line_to(x2, 15, 0, 0, 40, 2);
    planner_line_to(x3, 15, 0, 0, 40, 3);

    assert(moving == 1);

    while(moving)
    {
        moves_step_tick();
    }

    for (i = 0; i < 3; i++)
        assert(s[i] == x1[i] + x2[i] + x3[i]);
}

int main(void)
{
    test_multiple_lines();

    return 0;
}

