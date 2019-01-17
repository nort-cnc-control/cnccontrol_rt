#include "arc.h"
#include <math.h>
#include "common.h"
#include <moves.h>
#include <print.h>
#include <stdlib.h>
#include <shell.h>
#include <acceleration.h>

#include <assert.h>

#define SQR(a) ((a) * (a))

static steppers_definition def;
void arc_init(steppers_definition definition)
{
    def = definition;
}

static int32_t imaxx(int32_t a, int32_t b)
{
    double fa = a;
    double fb = b;
    double mx = SQR(fa) / sqrt(SQR(fa) + SQR(fb));
    return mx + 0.5;
}

static double fun2(int32_t x, int32_t a, int32_t b)
{
    double fa = a;
    double fb = b;
    double fx = x;
    return SQR(fb) * (1.0 - SQR(fx / fa));
}

static int32_t ifun(int32_t x, int32_t a, int32_t b)
{
    double fa = a;
    double fb = b;
    double fx = x;
    double fy = fb * sqrt(1.0 - SQR(fx / fa));
    return fy + 0.5;
}

// Running

static arc_plan *current_plan;

static struct
{
    double start[3];
    int32_t steps[3];
    int32_t dir[3];
    int segment_id;
    arc_segment *segment;
    int32_t x0;
    int32_t y0;
    int32_t x1;
    int32_t y1;
    int32_t a;
    int32_t b;
    int stx;
    int sty;
    int dx;
    int x;
    int y;
    double step_str;
    double step_hyp;

    acceleration_state acc;
} current_state;

static void start_segment(arc_segment *s)
{
    current_state.dir[0] = 0;
    current_state.dir[1] = 0;
    current_state.dir[2] = 0;
    current_state.segment = s;
    /*shell_send_string("seg: ");
    shell_print_dec(s->start[0]);
    shell_send_char(' ');
    shell_print_dec(s->start[1]);
    shell_send_char(' ');
    shell_print_dec(s->finish[0]);
    shell_send_char(' ');
    shell_print_dec(s->finish[1]);
    shell_send_char('\n');*/
    if (s->go_x)
    {
        current_state.x0 = s->start[0];
        current_state.y0 = s->start[1];
        current_state.x1 = s->finish[0];
        current_state.y1 = s->finish[1];
        current_state.a = s->a;
        current_state.b = s->b;
        current_state.stx = s->stx;
        current_state.sty = s->sty;
    }
    else
    {
        current_state.x0 = s->start[1];
        current_state.y0 = s->start[0];
        current_state.x1 = s->finish[1];
        current_state.y1 = s->finish[0];
        current_state.a = s->b;
        current_state.b = s->a;
        current_state.stx = s->sty;
        current_state.sty = s->stx;
    }

    if (current_state.x1 > current_state.x0)
        current_state.dx = 1;
    else
        current_state.dx = -1;

    current_state.dir[current_state.stx] = current_state.dx;

    current_state.x = current_state.x0;
    current_state.y = current_state.y0;

    double step_x = 1.0 / def.steps_per_unit[current_state.stx];
    double step_y = 1.0 / def.steps_per_unit[current_state.sty];

    current_state.step_str = step_x;
    current_state.step_hyp = sqrt(SQR(step_x) + SQR(step_y));

    def.set_dir(current_state.stx, current_state.dx >= 0);

    return;
}

// Returns length of step
static double make_step(void)
{
    if (current_state.x == current_state.x1)
    {
        return -1;
    }

    current_state.acc.step += 1;

    current_state.x += current_state.dx;
    def.make_step(current_state.stx);
    current_state.steps[current_state.stx] += current_state.dx;

    double fy = current_state.y;
    double y2 = fun2(current_state.x, current_state.a, current_state.b);
    if (abs(SQR(fy - 1) - y2) < abs(SQR(fy) - y2))
    {
        //shell_send_char('d');
        current_state.y -= 1;
        current_state.steps[current_state.sty] -= 1;
        current_state.dir[current_state.sty] = -1;
        def.set_dir(current_state.sty, 0);
        def.make_step(current_state.sty);
        return current_state.step_hyp;
    }
    else if (abs(SQR(fy + 1) - y2) < abs(SQR(fy) - y2))
    {
        //shell_send_char('i');
        current_state.y += 1;
        current_state.steps[current_state.sty] += 1;
        current_state.dir[current_state.sty] = 1;
        def.set_dir(current_state.sty, 1);
        def.make_step(current_state.sty);
        return current_state.step_hyp;
    }
    //shell_send_char('0');
    current_state.dir[current_state.sty] = 0;
    return current_state.step_str;
}

static double plan_tick()
{
    double len = make_step();

    // Set current position
    double cx[3];
    int i;
    for (i = 0; i < 3; i++)
    {
        cx[i] = current_state.start[i] + current_state.steps[i] / def.steps_per_unit[i];
    }
    moves_set_position(cx);

    if (current_state.x == current_state.x1)
    {
        /*shell_send_string("x = ");
        shell_print_dec(current_state.x);
        shell_send_string("(");
        shell_print_dec(current_state.x1);
        shell_send_string(") y = ");
        shell_print_dec(current_state.y);
        shell_send_string("(");
        shell_print_dec(current_state.y1);
        shell_send_string(")\n\r");*/
        assert(current_state.x == current_state.x1);
        assert(current_state.y == current_state.y1);

        // Segment is finished
        int seg = current_state.segment_id;
        seg++;
        if (seg < current_plan->num_segments)
        {
            current_state.segment_id = seg;
            start_segment(&current_plan->segments[seg]);
        }
        else
        {
            return -1;
        }
    }

    return len;
}

// module API functions

int arc_step_tick(void)
{
    // Make step
    double len = plan_tick();

    // Check if we have reached the end
    if (len <= 0)
    {
        /*shell_send_string("Real total: ");
        shell_print_dec(current_state.acc.step);
        shell_send_string("\n\r");*/
        def.line_finished();
        return -1;
    }

    // Check for endstops
    if (current_plan->check_break && current_plan->check_break(current_state.dir, current_plan->check_break_data))
    {
        shell_send_string("debug: break\n\r");
        def.line_finished();
        return -1;
    }

    // Calculate delay
    int step_delay = feed2delay(current_state.acc.feed, current_plan->len / current_plan->steps);
    acceleration_process(&current_state.acc, step_delay);
    return step_delay;
}

int arc_move_to(arc_plan *plan)
{
    current_plan = plan;
    if (!plan->ready)
    {
        arc_pre_calculate(plan);
    }
    int i;
    for (i = 0; i < 3; i++)
    {
        current_state.start[i] = position.pos[i];
        current_state.steps[i] = 0;
    }

    current_state.acc.acceleration = current_plan->acceleration;
    current_state.acc.feed = current_plan->feed0;
    current_state.acc.target_feed = current_plan->feed;
    current_state.acc.end_feed = current_plan->feed1;
    current_state.acc.type = STATE_ACC;
    current_state.acc.step = 0;
    current_state.acc.total_steps = current_plan->steps;
    current_state.acc.acc_steps = current_plan->acc_steps;
    current_state.acc.dec_steps = current_plan->dec_steps;
    
    current_state.segment_id = 0;
    start_segment(&(current_plan->segments[0]));
    def.line_started();
    return -E_OK;
}

// Pre-calculating

static int build_arc_segment(arc_segment *segment, int32_t sx, int32_t sy, int32_t ex, int32_t ey, int32_t x_s, int32_t y_s, int32_t a, int32_t b)
{
    segment->go_x = 0;
    segment->go_y = 0;
    segment->sign = 0;
    segment->a = a;
    segment->b = b;

    segment->start[0] = sx;
    segment->start[1] = sy;
    segment->finish[0] = ex;
    segment->finish[1] = ey;

    if (sx <= -x_s && ex <= -x_s)
    {
        segment->go_y = 1;
        segment->sign = -1;
    }
    else if (sx >= x_s && ex >= x_s)
    {
        segment->go_y = 1;
        segment->sign = 1;
    }
    else if (sy <= -y_s && ey <= -y_s)
    {
        segment->go_x = 1;
        segment->sign = -1;
    }
    else if (sy >= y_s && ey >= y_s)
    {
        segment->go_x = 1;
        segment->sign = 1;
    }
    else
    {
        // Error!
    }
    if (segment->go_x)
        return abs(ex - sx);
    else if (segment->go_y)
        return abs(ey - sy);
    return -1;
}

static int build_arc_bottom_ccw(arc_plan *arc, int32_t x_s, int32_t y_s, int32_t ex, int32_t ey, int32_t a, int32_t b)
{
    if (ey <= -y_s)
    {
        int steps = build_arc_segment(&arc->segments[arc->num_segments++], -x_s, -y_s, ex, ey, x_s, y_s, a, b);
        arc->steps += steps;
        return 1;
    }
    else
    {
        int steps = build_arc_segment(&arc->segments[arc->num_segments++], -x_s, -y_s, x_s, -y_s, x_s, y_s, a, b);
        arc->steps += steps;
        return 0;
    }
}

static int build_arc_top_ccw(arc_plan *arc, int32_t x_s, int32_t y_s, int32_t ex, int32_t ey, int32_t a, int32_t b)
{
    if (ey >= y_s)
    {
        int steps = build_arc_segment(&arc->segments[arc->num_segments++], x_s, y_s, ex, ey, x_s, y_s, a, b);
        arc->steps += steps;
        return 1;
    }
    else
    {
        int steps = build_arc_segment(&arc->segments[arc->num_segments++], x_s, y_s, -x_s, y_s, x_s, y_s, a, b);
        arc->steps += steps;
        return 0;
    }
}

static int build_arc_right_ccw(arc_plan *arc, int32_t x_s, int32_t y_s, int32_t ex, int32_t ey, int32_t a, int32_t b)
{
    if (ex >= x_s)
    {
        int steps = build_arc_segment(&arc->segments[arc->num_segments++], x_s, -y_s, ex, ey, x_s, y_s, a, b);
        arc->steps += steps;
        return 1;
    }
    else
    {
        int steps = build_arc_segment(&arc->segments[arc->num_segments++], x_s, -y_s, x_s, y_s, x_s, y_s, a, b);
        arc->steps += steps;
        return 0;
    }
}

static int build_arc_left_ccw(arc_plan *arc, int32_t x_s, int32_t y_s, int32_t ex, int32_t ey, int32_t a, int32_t b)
{
    if (ex <= -x_s)
    {
        int steps = build_arc_segment(&arc->segments[arc->num_segments++], -x_s, y_s, ex, ey, x_s, y_s, a, b);
        arc->steps += steps;
        return 1;
    }
    else
    {
        int steps = build_arc_segment(&arc->segments[arc->num_segments++], -x_s, y_s, -x_s, -y_s, x_s, y_s, a, b);
        arc->steps += steps;
        return 0;
    }
}

static void make_arc_ccw(arc_plan *arc, int32_t sx, int32_t sy, int32_t ex, int32_t ey, int32_t a, int32_t b)
{
    arc->num_segments = 0;
    int32_t x_s = imaxx(a, b);
    int32_t y_s = ifun(x_s, a, b);

    if (sx >= x_s)
    {
        // Start from right
        if (ex >= x_s && ey >= sy)
        {
            int steps = build_arc_segment(&arc->segments[arc->num_segments++], sx, sy, ex, ey, x_s, y_s, a, b);
            arc->steps += steps;
            return;
        }
        else
        {
            int steps = build_arc_segment(&arc->segments[arc->num_segments++], sx, sy, x_s, y_s, x_s, y_s, a, b);
            arc->steps += steps;
        }

        if (build_arc_top_ccw(arc, x_s, y_s, ex, ey, a, b))
            return;
        if (build_arc_left_ccw(arc, x_s, y_s, ex, ey, a, b))
            return;
        if (build_arc_bottom_ccw(arc, x_s, y_s, ex, ey, a, b))
            return;

        int steps = build_arc_segment(&arc->segments[arc->num_segments++], x_s, -y_s, ex, ey, x_s, y_s, a, b);
        arc->steps += steps;
        return;
    }
    if (sx <= -x_s)
    {
        // Start from left
        if (ex <= -x_s && ey <= sy)
        {
            int steps = build_arc_segment(&arc->segments[arc->num_segments++], sx, sy, ex, ey, x_s, y_s, a, b);
            arc->steps += steps;
            return;
        }
        else
        {
            int steps = build_arc_segment(&arc->segments[arc->num_segments++], sx, sy, -x_s, -y_s, x_s, y_s, a, b);
            arc->steps += steps;
        }

        if (build_arc_bottom_ccw(arc, x_s, y_s, ex, ey, a, b))
            return;
        if (build_arc_right_ccw(arc, x_s, y_s, ex, ey, a, b))
            return;
        if (build_arc_top_ccw(arc, x_s, y_s, ex, ey, a, b))
            return;

        int steps = build_arc_segment(&arc->segments[arc->num_segments++], -x_s, y_s, ex, ey, x_s, y_s, a, b);
        arc->steps += steps;
        return;
    }
    if (sy >= y_s)
    {
        // Start from top
        if (ey >= y_s && ex <= sx)
        {
            int steps = build_arc_segment(&arc->segments[arc->num_segments++], sx, sy, ex, ey, x_s, y_s, a, b);
            arc->steps += steps;
            return;
        }
        else
        {
            int steps = build_arc_segment(&arc->segments[arc->num_segments++], sx, sy, -x_s, y_s, x_s, y_s, a, b);
            arc->steps += steps;
        }

        if (build_arc_left_ccw(arc, x_s, y_s, ex, ey, a, b))
            return;
        if (build_arc_bottom_ccw(arc, x_s, y_s, ex, ey, a, b))
            return;
        if (build_arc_right_ccw(arc, x_s, y_s, ex, ey, a, b))
            return;

        int steps = build_arc_segment(&arc->segments[arc->num_segments++], x_s, y_s, ex, ey, x_s, y_s, a, b);
        arc->steps += steps;
        return;
    }
    if (sy <= -y_s)
    {
        // Start from bottom
        if (ey <= -y_s && ex >= sx)
        {
            int steps = build_arc_segment(&arc->segments[arc->num_segments++], sx, sy, ex, ey, x_s, y_s, a, b);
            arc->steps += steps;
            return;
        }
        else
        {
            int steps = build_arc_segment(&arc->segments[arc->num_segments++], sx, sy, x_s, -y_s, x_s, y_s, a, b);
            arc->steps += steps;
        }

        if (build_arc_right_ccw(arc, x_s, y_s, ex, ey, a, b))
            return;
        if (build_arc_top_ccw(arc, x_s, y_s, ex, ey, a, b))
            return;
        if (build_arc_left_ccw(arc, x_s, y_s, ex, ey, a, b))
            return;

        int steps = build_arc_segment(&arc->segments[arc->num_segments++], -x_s, -y_s, ex, ey, x_s, y_s, a, b);
        arc->steps += steps;
        return;
    }

    // Error!
}

static void arc_segment_reverse(arc_segment *a)
{
    int32_t sx = a->start[0];
    int32_t sy = a->start[1];
    a->start[0] = a->finish[0];
    a->start[1] = a->finish[1];
    a->finish[0] = sx;
    a->finish[1] = sy;
}

static void make_arc_cw(arc_plan *arc, int32_t sx, int32_t sy, int32_t ex, int32_t ey, int32_t a, int32_t b)
{
    int i;
    make_arc_ccw(arc, ex, ey, sx, sy, a, b);
    for (i = 0; i <= arc->num_segments / 2; i++)
    {
        arc_segment t = arc->segments[i];
        arc->segments[i] = arc->segments[arc->num_segments - i - 1];
        arc->segments[arc->num_segments - i - 1] = t;
    }
    for (i = 0; i < arc->num_segments; i++)
    {
        arc_segment_reverse(&(arc->segments[i]));
    }
}

void arc_pre_calculate(arc_plan *arc)
{
    double delta[2];

    int stx, sty;
    double d = arc->d;
    int cw = arc->cw;
    
    switch (arc->plane)
    {
    case XY:
        delta[0] = arc->x[0];
        delta[1] = arc->x[1];
        stx = 0;
        sty = 1;
        if (!def.xy_right)
        {
            cw = !cw;
            d = -d;
        }
        break;
    case YZ:
        delta[0] = arc->x[1];
        delta[1] = arc->x[2];
        stx = 1;
        sty = 2;
        if (!def.yz_right)
        {
            cw = !cw;
            d = -d;
        }
        break;
    case ZX:
        delta[0] = arc->x[2];
        delta[1] = arc->x[0];
        stx = 2;
        sty = 0;
        if (!def.zx_right)
        {
            cw = !cw;
            d = -d;
        }
        break;
    }
    double len = sqrt(SQR(delta[0]) + SQR(delta[1]));
    double p[2] = {delta[1] / len, -delta[0] / len}; // ortogonal vector

    double center[2] = {delta[0] / 2 + p[0] * d, delta[1] / 2 + p[1] * d};
    double radius = sqrt(SQR(len) / 4 + SQR(d));

    int32_t start[2];
    int32_t finish[2];
    int32_t a, b;
    
    switch (arc->plane)
    {
    case XY:
        a = radius * def.steps_per_unit[0];
        b = radius * def.steps_per_unit[1];
        start[0] = -center[0] * def.steps_per_unit[0];
        start[1] = -center[1] * def.steps_per_unit[1];
        finish[0] = (delta[0] - center[0]) * def.steps_per_unit[0];
        finish[1] = (delta[1] - center[1]) * def.steps_per_unit[1];
        break;
    case YZ:
        a = radius * def.steps_per_unit[1];
        b = radius * def.steps_per_unit[2];
        start[0] = -center[0] * def.steps_per_unit[1];
        start[1] = -center[1] * def.steps_per_unit[2];
        finish[0] = (delta[0] - center[0]) * def.steps_per_unit[1];
        finish[1] = (delta[1] - center[1]) * def.steps_per_unit[2];
        break;
    case ZX:
        a = radius * def.steps_per_unit[2];
        b = radius * def.steps_per_unit[0];
        start[0] = -center[0] * def.steps_per_unit[2];
        start[1] = -center[1] * def.steps_per_unit[0];
        finish[0] = (delta[0] - center[0]) * def.steps_per_unit[2];
        finish[1] = (delta[1] - center[1]) * def.steps_per_unit[0];
        break;
    }

    if (cw)
    {
        make_arc_cw(arc, start[0], start[1], finish[0], finish[1], a, b);
    }
    else
    {
        make_arc_ccw(arc, start[0], start[1], finish[0], finish[1], a, b);
    }

    int i;
    for (i = 0; i < arc->num_segments; i++)
    {
        arc->segments[i].stx = stx;
        arc->segments[i].sty = sty;
    }

    int total_steps = arc->steps;
    double cosa = (-center[0] * (delta[0] - center[0]) - center[1] * (delta[1] - center[1])) / (radius * radius);
    double angle = acos(cosa);
    if (cw)
    {
        if (arc->d > 0)
        {
            // small arc
        }
        else
        {
            // big arc
            angle = 2*3.14159265358 - angle;
        }
    }
    else
    {
        if (arc->d > 0)
        {
            // big arc
            angle = 2*3.14159265358 - angle;
        }
        else
        {
            // small arc
        }
    }
    arc->len = radius * angle;
    arc->acc_steps = acceleration_steps(arc->feed0, arc->feed, arc->acceleration, arc->len, arc->steps);
    arc->dec_steps = acceleration_steps(arc->feed1, arc->feed, arc->acceleration, arc->len, arc->steps);
    if (arc->acc_steps + arc->dec_steps > arc->steps)
    {
        int32_t d = (arc->acc_steps + arc->dec_steps - arc->steps) / 2;
        arc->acc_steps -= d;
        arc->dec_steps -= d;
        if (arc->acc_steps + arc->dec_steps < arc->steps)
            arc->acc_steps += (arc->steps - arc->acc_steps - arc->dec_steps);
    }
    arc->ready = 1;
}
