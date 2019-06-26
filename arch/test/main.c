#include <stdio.h>
#include <arc.h>
#include <planner.h>
#include <control.h>
#include <shell_print.h>
#include <shell_read.h>
#include <rdpos_io.h>
#include "config.h"
#include <math.h>
#include <pthread.h>
#include <termios.h>
#include <fcntl.h>

int dsteps[3] = {0, 0, 0};
int steps[3];
double pos[3];

static void set_dir(int coord, int dir)
{
    if (dir == 0)
        dsteps[coord] = -1;
    else
        dsteps[coord] = 1;
}

static void make_step(int coord)
{
    steps[coord] += dsteps[coord];
    pos[coord] = steps[coord] / STEPS_PER_MM;
}

static cnc_endstops get_stops(void)
{
    cnc_endstops stops = {
        .stop_x  = 0,
        .stop_y  = 0,
        .stop_z  = 0,
        .probe_z = 0,
    };

    return stops;
}

static void line_started(void)
{
    printf("Line started\n");
}

static void line_finished(void)
{
    printf("Line finished\n");
}

static void line_error(void)
{
    printf("Line error\n");
}

void config_steppers(steppers_definition *sd)
{
    sd->set_dir        = set_dir;
    sd->make_step      = make_step;
    sd->get_endstops   = get_stops;
    sd->line_started   = line_started;
    sd->line_finished  = line_finished;
    sd->line_error     = line_error;
}

static void init_steppers(void)
{
    steppers_definition sd = {
        .steps_per_unit = {
            STEPS_PER_MM,
            STEPS_PER_MM,
            STEPS_PER_MM
        },
        .feed_base = FEED_BASE,
        .feed_max = FEED_MAX,
        .es_travel = FEED_ES_TRAVEL,
        .probe_travel = FEED_PROBE_TRAVEL,
        .es_precise = FEED_ES_PRECISE,
        .probe_precise = FEED_PROBE_PRECISE,
        .size = {
            SIZE_X,
            SIZE_Y,
            SIZE_Z,
        },
        .acc_default = ACC,
    };
    config_steppers(&sd);
    init_control(sd);
}

void test_init(void)
{
    pos[0] = 0;
    pos[1] = 0;
    pos[2] = 0;
    steps[0] = 0;
    steps[1] = 0;
    steps[2] = 0;
}

pthread_cond_t connected_flag;

static struct serial_cbs_s *serial_cbs = &rdpos_io_serial_cbs;
static struct shell_cbs_s  *shell_cbs =  &rdpos_io_shell_cbs;

static int fd;
static int clsd = 0;

static void transmit_char(uint8_t c)
{
    write(fd, &c, 1);
    serial_cbs->byte_transmitted();
}

void *receive(void *arg)
{
    printf("Starting recv thread. fd = %i\n", fd);
    while (!clsd)
    {
        unsigned char b;
        ssize_t n = read(fd, &b, 1);
        if (n < 1)
        {
            usleep(100);
            continue;
        }
        serial_cbs->byte_received(b);
    }
    return NULL;
}

pthread_t tid; /* идентификатор потока */

int main(int argc, const char **argv)
{
    const char *serial_port = argv[1];
    printf("Opening %s\n", serial_port);
    
    fd = open(serial_port, O_RDWR | O_NOCTTY | O_SYNC | O_NDELAY);
    if (fd <= 0)
    {
        printf("Error opening\n");
        return 0;
    }
    printf("Serial opened. fd = %i\n", fd);
    rdpos_io_init();

    shell_print_init(&rdpos_io_shell_cbs);
    shell_read_init(&rdpos_io_shell_cbs);
 	
    serial_cbs->register_byte_transmit(transmit_char);

    pthread_create(&tid, NULL, receive, &fd);
    usleep(100000);

    test_init();
    init_steppers();
	
    while (true)
    {
        while (!shell_connected())
            usleep(1000);
        printf("Connected\n");
        planner_lock();
        moves_reset();
        shell_send_string("Hello");
        while (shell_connected())
        {
            planner_pre_calculate();
            usleep(1000);
        }
        printf("Disconnected\n");
    }
    
	return 0;
}
