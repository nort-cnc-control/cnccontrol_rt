#include <math.h>
#include <pthread.h>
#include <termios.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "config.h"

#include <control/planner/planner.h>
#include <control/control.h>
#include <output/output.h>
#include <control/commands/gcode_handler/gcode_handler.h>

#include <control/moves/moves.h>
#include <control/commands/status/print_status.h>

static void print_pos(void);


int dsteps[3] = {0, 0, 0};
int steps[3];
double pos[3];
bool run = false;

static void set_dir(int coord, bool dir)
{
    if (dir == false)
        dsteps[coord] = 1;
    else
        dsteps[coord] = -1;
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
        .probe = 0,
    };

    return stops;
}

bool line_st;
pthread_t tid_tick; /* идентификатор потока */


static void* make_tick(void *arg)
{
    while (line_st)
    {
        int delay_us = moves_step_tick();
        if (delay_us <= 0)
        {
            break;
        }
        usleep(delay_us);
    }
    return NULL;
}

static void line_started(void)
{
    printf("Line started\n");
    print_pos();
    line_st = true;
    pthread_create(&tid_tick, NULL, make_tick, NULL);
}

static void line_finished(void)
{
    printf("Line finished\n");
    line_st = false;
    pthread_join(tid_tick, NULL);
    print_pos();
}

static void line_error(void)
{
    printf("Line error\n");
    line_st = false;
    pthread_join(tid_tick, NULL);
    print_pos();
}


static void print_pos(void)
{
    int x, y, z;
    x = position.pos[0];
    y = position.pos[1];
    z = position.pos[2];

    printf("Position = %i %i %i\n", x, y, z);
}

static void reboot(void)
{
    planner_lock();
    steps[0] = 0;
    steps[1] = 0;
    steps[2] = 0;
    pos[0] = 0;
    pos[1] = 0;
    pos[2] = 0;
    moves_reset();
    output_control_write("Hello", -1);
    printf("Reboot\n");
}

static void set_gpio(int i, int on)
{
    if (on)
        printf("Tool %i in on\n", i);
    else
        printf("Tool %i is off\n", i);
}

void config_steppers(steppers_definition *sd, gpio_definition *gd)
{
    sd->reboot         = reboot;
    sd->set_dir        = set_dir;
    sd->make_step      = make_step;
    sd->get_endstops   = get_stops;
    sd->line_started   = line_started;
    sd->line_finished  = line_finished;
    sd->line_error     = line_error;
    gd->set_gpio       = set_gpio;
}

static void init_steppers(void)
{
    gpio_definition gd;

    steppers_definition sd = {
        .steps_per_unit = {
            STEPS_PER_MM,
            STEPS_PER_MM,
            STEPS_PER_MM
        },
        .feed_base = FEED_BASE / 60.0,
        .feed_max = FEED_MAX / 60.0,
        .es_travel = FEED_ES_TRAVEL / 60.0,
        .probe_travel = FEED_PROBE_TRAVEL / 60.0,
        .es_precise = FEED_ES_PRECISE / 60.0,
        .probe_precise = FEED_PROBE_PRECISE / 60.0,
        .size = {
            SIZE_X,
            SIZE_Y,
            SIZE_Z,
        },
    };
    config_steppers(&sd, &gd);
    init_control(&sd, &gd);
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

pthread_mutex_t mutex;
static int fd;

void *receive(void *arg)
{
    static char buf[1000];
    size_t blen = 0;

    printf("Starting recv thread. fd = %i\n", fd);
    while (run)
    {
        unsigned char b;
        ssize_t n = read(fd, &b, 1);


        if (n < 1)
        {
            printf("Disconnected\n");
            run = false;
            break;
        }
        if (b == '\n' || b == '\r')
        {
            if (blen >= 3 && !memcmp(buf, "RT:", 3))
            {
                pthread_mutex_lock(&mutex);
                printf("Execute: %.*s\n", (int)(blen - 3), buf + 3);
                execute_g_command((const unsigned char*)(buf + 3), blen - 3);
                pthread_mutex_unlock(&mutex);
            }
            else if (blen >= 5 && !memcmp(buf, "EXIT:", 5))
            {
                run = false;
            }
            blen = 0;
            memset(buf, 0, sizeof(buf));
        }
        else
        {
            buf[blen++] = b;
        }
    }
    return NULL;
}

static ssize_t write_fun(int fd, const void *data, ssize_t len)
{
//    printf("Send line: %.*s\n", len, (const char*)data);
    if (fd == 0)
    {
        write(fd, data, len);
        write(fd, "\n", 1);
    }
    else
    {
        write(fd, data, len);
        write(fd, "\n", 1);
    }
    return 0;
}

static int create_control(unsigned short port)
{
    struct sockaddr_in ctl_sockaddr;
    int ctlsock = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(ctlsock, SOL_SOCKET, SO_REUSEADDR, NULL, 0);

    ctl_sockaddr.sin_family = AF_INET;
    ctl_sockaddr.sin_port = htons(port);
    ctl_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(ctlsock, (struct sockaddr *)&ctl_sockaddr, sizeof(ctl_sockaddr)) < 0)
    {
        printf("Can not bind tcp\n");
        return -1;
    }
    listen(ctlsock, 1);
    return ctlsock;
}

int main(int argc, const char **argv)
{
    pthread_t tid_rcv; /* идентификатор потока */
    const int port = CONFIG_TCP_PORT;
    
    int sock = create_control(port);
    if (sock <= 0)
    {
        printf("Error opening\n");
        return 0;
    }

    printf("Listening control on :%i\n", port);
    pthread_mutex_init(&mutex, NULL);

    while (true)
    {
        fd = accept(sock, NULL, NULL);
        run = true;
        printf("Connect from client\n");


        pthread_create(&tid_rcv, NULL, receive, &fd);
        usleep(100000);

        output_control_set_fd(fd);
        output_shell_set_fd(0);
        output_set_write_fun(write_fun);

        test_init();
        init_steppers();

        output_control_write("Hello", -1);

        planner_lock();
        moves_reset();

        while (run)
        {
            pthread_mutex_lock(&mutex);
            planner_pre_calculate();
            pthread_mutex_unlock(&mutex);
            usleep(1000);
        }

        close(fd);
    }
    pthread_mutex_destroy(&mutex);
    return 0;
}
