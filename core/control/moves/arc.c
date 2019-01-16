#include "arc.h"
#include <math.h>
#include "common.h"

#include <moves.h>
#include <print.h>
#include <shell.h>

static const fixed one = FIXED_ENCODE(1);

static steppers_definition def;
void arc_init ( steppers_definition definition )
{
    def = definition;
}

static int32_t imaxx(int32_t a, int32_t b)
{
    fixed fa = FIXED_ENCODE(a);
    fixed fb = FIXED_ENCODE(b);
    fixed mx = DIV(SQR(fa), fsqrt(SQR(fa) + SQR(fb)));
    return FIXED_DECODE_ROUND(mx);
}

static fixed fun2(int32_t x, int32_t a, int32_t b)
{
    fixed fa = FIXED_ENCODE(a);
    fixed fb = FIXED_ENCODE(b);
    fixed fx = FIXED_ENCODE(x);
    return MUL(SQR(fb), one - SQR(DIV(fx, fa)));
}

static int32_t ifun(int32_t x, int32_t a, int32_t b)
{
    fixed fa = FIXED_ENCODE(a);
    fixed fb = FIXED_ENCODE(b);
    fixed fx = FIXED_ENCODE(x);
    fixed fy = MUL(fb, fsqrt(one - SQR(DIV(fx, fa))));
    return FIXED_DECODE_ROUND(fy);
}

// Running

static arc_plan *current_plan;

static struct
{
    fixed start[3];
    int32_t steps[3];
    int32_t dir[3];
    fixed feed;
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
    fixed step_str;
    fixed step_hyp;
} current_state;

static void start_segment(arc_segment *s)
{
    /*shell_send_string("start segment\n\r");
    shell_print_dec(s->start[0]);
    shell_send_string("\n\r");
    shell_print_dec(s->start[1]);
    shell_send_string("\n\r");
    shell_print_dec(s->finish[0]);
    shell_send_string("\n\r");
    shell_print_dec(s->finish[1]);
    shell_send_string("\n\r");
    shell_send_string("*****\n\r");*/
    current_state.dir[0] = 0;
    current_state.dir[1] = 0;
    current_state.dir[2] = 0;
    current_state.segment = s;
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

    fixed step_x = one / def.steps_per_unit[current_state.stx];
    fixed step_y = one / def.steps_per_unit[current_state.sty];
    
    current_state.step_str = step_x;
    current_state.step_hyp = fsqrt(SQR(step_x) + SQR(step_y));

    def.set_dir(current_state.stx, current_state.dx >= 0);

    /*shell_send_string("debug: str len = ");
    shell_print_fixed(current_state.step_str*100);
    shell_send_string("\n\r");

    shell_send_string("debug: hyp len = ");
    shell_print_fixed(current_state.step_hyp*100);
    shell_send_string("\n\r");
*/
    return;
}

// Returns length of step
static fixed make_step(void)
{
    if (current_state.x == current_state.x1)
    {
        shell_send_string("debug: unexpected finish\n\r");
        return -1;
    }
    current_state.x += current_state.dx;
    def.make_step(current_state.stx);
    current_state.steps[current_state.stx] += current_state.dx;

    fixed fy = FIXED_ENCODE(current_state.y);
    fixed y2 = fun2(current_state.x, current_state.a, current_state.b);
    if (abs( SQR(fy - one) - y2) < abs(SQR(fy) - y2))
    {
        //shell_send_char('d');
        current_state.y -= 1;
        current_state.steps[current_state.sty] -= 1;
        current_state.dir[current_state.sty] = -1;
        def.set_dir(current_state.sty, 0);
        def.make_step(current_state.sty);
        return current_state.step_str;
    }
    else if (abs( SQR(fy + one) - y2) < abs(SQR(fy) - y2))
    {
        //shell_send_char('i');
        current_state.y += 1;
        current_state.steps[current_state.sty] += 1;
        current_state.dir[current_state.sty] = 1;
        def.set_dir(current_state.sty, 1);
        def.make_step(current_state.sty);
        return current_state.step_str;
    }
    //shell_send_char('0');
    current_state.dir[current_state.sty] = 0;
    return current_state.step_str;
}


static fixed plan_tick()
{
    int i;
    fixed len = make_step();
    for (i = 0; i < 3; i++)
    {
        shell_print_dec(current_state.steps[i]);
        shell_send_char(' ');
    }
    shell_send_string("\r\n");

    if (current_state.x == current_state.x1)
    {
	shell_send_string("debug: finish segment: ");
        /*for (i = 0; i < 3; i++)
        {
            shell_print_dec(current_state.steps[i]);
            shell_send_char(' ');
        }
	shell_send_string("\r\n");
*/
        //shell_send_string("debug: segment is finished\n\r");
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
    fixed len = plan_tick();

    fixed cx[3];
    int i;
    for (i = 0; i < 3; i++)
    {
        cx[i] = current_state.start[i] + FIXED_ENCODE(current_state.steps[i]) / def.steps_per_unit[i];
    }
    moves_set_position(cx);

    if (len <= 0)
    {
        shell_send_string("debug: finish: ");
        for (i = 0; i < 3; i++)
        {
            shell_print_dec(current_state.steps[i]);
            shell_send_char(' ');
        }
        shell_send_string("\n\r");
        //shell_send_string("debug: arc finished\n\r");
        def.line_finished();
        return -1;
    }

    if (current_plan->check_break && current_plan->check_break(current_state.dir, current_plan->check_break_data))
    {
        shell_send_string("debug: break\n\r");
        def.line_finished();
        return -1;
    }


    int step_delay = feed2delay(current_state.feed, len, 1);
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
    
    /*shell_send_string("debug: num segments: ");
    shell_print_dec(current_plan->num_segments);
    shell_send_string("\n\r");*/
    current_state.segment_id = 0;
    start_segment(&(current_plan->segments[0]));
    current_state.feed = current_plan->feed;
    def.line_started();
    return -E_OK;
}

// Pre-calculating

static void build_arc_segment(arc_segment *segment, int32_t sx, int32_t sy, int32_t ex, int32_t ey, int32_t x_s, int32_t y_s, int32_t a, int32_t b)
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
}

static int build_arc_bottom_ccw(arc_plan *arc, int32_t x_s, int32_t y_s, int32_t ex, int32_t ey, int32_t a, int32_t b)
{
    if (ey <= -y_s)
    {
        build_arc_segment(&arc->segments[arc->num_segments++], -x_s, -y_s, ex, ey, x_s, y_s, a, b);
        return 1;
    }
    else
    {
        build_arc_segment(&arc->segments[arc->num_segments++], -x_s, -y_s, x_s, -y_s, x_s, y_s, a, b);
        return 0;
    }
}

static int build_arc_top_ccw(arc_plan *arc, int32_t x_s, int32_t y_s, int32_t ex, int32_t ey, int32_t a, int32_t b)
{
    if (ey >= y_s)
    {
        build_arc_segment(&arc->segments[arc->num_segments++], x_s, y_s, ex, ey, x_s, y_s, a, b);
        return 1;
    }
    else
    {
        build_arc_segment(&arc->segments[arc->num_segments++], x_s, y_s, -x_s, y_s, x_s, y_s, a, b);
        return 0;
    }
}

static int build_arc_right_ccw(arc_plan *arc, int32_t x_s, int32_t y_s, int32_t ex, int32_t ey, int32_t a, int32_t b)
{
    if (ex >= x_s)
    {
        build_arc_segment(&arc->segments[arc->num_segments++], x_s, -y_s, ex, ey, x_s, y_s, a, b);
        return 1;
    }
    else
    {
        build_arc_segment(&arc->segments[arc->num_segments++], x_s, -y_s, x_s, y_s, x_s, y_s, a, b);
        return 0;
    }
}

static int build_arc_left_ccw(arc_plan *arc, int32_t x_s, int32_t y_s, int32_t ex, int32_t ey, int32_t a, int32_t b)
{
    if (ex <= -x_s)
    {
        build_arc_segment(&arc->segments[arc->num_segments++], -x_s, y_s, ex, ey, x_s, y_s, a, b);
        return 1;
    }
    else
    {
        build_arc_segment(&arc->segments[arc->num_segments++], -x_s, y_s, -x_s, -y_s, x_s, y_s, a, b);
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
            build_arc_segment(&arc->segments[arc->num_segments++], sx, sy, ex, ey, x_s, y_s, a, b);
            return;
        }
        else
        {
            build_arc_segment(&arc->segments[arc->num_segments++], sx, sy, x_s, y_s, x_s, y_s, a, b);
        }

        if (build_arc_top_ccw(arc, x_s, y_s, ex, ey, a, b))
            return;
        if (build_arc_left_ccw(arc, x_s, y_s, ex, ey, a, b))
            return;
        if (build_arc_bottom_ccw(arc, x_s, y_s, ex, ey, a, b))
            return;
        
        build_arc_segment(&arc->segments[arc->num_segments++], x_s, -y_s, ex, ey, x_s, y_s, a, b);
        return;
    }
    if (sx <= -x_s)
    {
        // Start from left
        if (ex <= -x_s && ey <= sy)
        {    
            build_arc_segment(&arc->segments[arc->num_segments++], sx, sy, ex, ey, x_s, y_s, a, b);
            return;
        }
        else
        {
            build_arc_segment(&arc->segments[arc->num_segments++], sx, sy, -x_s, -y_s, x_s, y_s, a, b);
        }

        if (build_arc_bottom_ccw(arc, x_s, y_s, ex, ey, a, b))
            return;
        if (build_arc_right_ccw(arc, x_s, y_s, ex, ey, a, b))
            return;
        if (build_arc_top_ccw(arc, x_s, y_s, ex, ey, a, b))
            return;
        
        build_arc_segment(&arc->segments[arc->num_segments++], -x_s, y_s, ex, ey, x_s, y_s, a, b);
        return;
    }
    if (sy >= y_s)
    {
        // Start from top
        if (ey >= y_s && ex <= sx)
        {    
            build_arc_segment(&arc->segments[arc->num_segments++], sx, sy, ex, ey, x_s, y_s, a, b);
            return;
        }
        else
        {
            build_arc_segment(&arc->segments[arc->num_segments++], sx, sy, -x_s, y_s, x_s, y_s, a, b);
        }

        if (build_arc_left_ccw(arc, x_s, y_s, ex, ey, a, b))
            return;
        if (build_arc_bottom_ccw(arc, x_s, y_s, ex, ey, a, b))
            return;
        if (build_arc_right_ccw(arc, x_s, y_s, ex, ey, a, b))
            return;
        
        build_arc_segment(&arc->segments[arc->num_segments++], x_s, y_s, ex, ey, x_s, y_s, a, b);
        return;
    }
    if (sy <= -y_s)
    {
        // Start from bottom
        if (ey <= -y_s && ex >= sx)
        {    
            build_arc_segment(&arc->segments[arc->num_segments++], sx, sy, ex, ey, x_s, y_s, a, b);
            return;
        }
        else
        {
            build_arc_segment(&arc->segments[arc->num_segments++], sx, sy, x_s, -y_s, x_s, y_s, a, b);
        }

        if (build_arc_right_ccw(arc, x_s, y_s, ex, ey, a, b))
            return;
        if (build_arc_top_ccw(arc, x_s, y_s, ex, ey, a, b))
            return;
        if (build_arc_left_ccw(arc, x_s, y_s, ex, ey, a, b))
            return;
        
        build_arc_segment(&arc->segments[arc->num_segments++], -x_s, -y_s, ex, ey, x_s, y_s, a, b);
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

static void make_arc_cw( arc_plan *arc, int32_t sx, int32_t sy, int32_t ex, int32_t ey, int32_t a, int32_t b)
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

void arc_pre_calculate ( arc_plan *arc )
{
    fixed delta[2];

    int stx, sty;

    switch (arc->plane)
    {
        case XY:
            delta[0] = arc->x[0];
            delta[1] = arc->x[1];
            stx = 0;
            sty = 1;
            break;
        case YZ:
            delta[0] = arc->x[1];
            delta[1] = arc->x[2];
            stx = 1;
            sty = 2;
            break;
        case ZX:
            delta[0] = arc->x[2];
            delta[1] = arc->x[0];
            stx = 2;
            sty = 0;
            break;
    }
    fixed len = fsqrt(SQR(delta[0]) + SQR(delta[1]));
    fixed p[2] = { DIV(delta[1], len), -DIV(delta[0], len) }; // ortogonal vector
    
    fixed center[2] = { delta[0]/2 + MUL(p[0], arc->d), delta[1]/2 + MUL(p[1], arc->d) };
    fixed radius = fsqrt(SQR(len)/4 + SQR(arc->d));

    int32_t start[2];
    int32_t finish[2];
    int32_t a, b;
    int cw = arc->cw;
    switch (arc->plane)
    {
        case XY:
            a = FIXED_DECODE(radius * def.steps_per_unit[0]);
            b = FIXED_DECODE(radius * def.steps_per_unit[1]);
            start[0] = FIXED_DECODE(-center[0] * def.steps_per_unit[0]);
            start[1] = FIXED_DECODE(-center[1] * def.steps_per_unit[1]);
            finish[0] = FIXED_DECODE((delta[0] - center[0]) * def.steps_per_unit[0]);
            finish[1] = FIXED_DECODE((delta[1] - center[1]) * def.steps_per_unit[1]);
            if (!def.xy_right)
                cw = !cw;
            break;
        case YZ:
            a = FIXED_DECODE(radius * def.steps_per_unit[1]);
            b = FIXED_DECODE(radius * def.steps_per_unit[2]);
            start[0] = FIXED_DECODE(-center[0] * def.steps_per_unit[1]);
            start[1] = FIXED_DECODE(-center[1] * def.steps_per_unit[2]);
            finish[0] = FIXED_DECODE((delta[0] - center[0]) * def.steps_per_unit[1]);
            finish[1] = FIXED_DECODE((delta[1] - center[1]) * def.steps_per_unit[2]);
            if (!def.yz_right)
                cw = !cw;
            break;
        case ZX:
            a = FIXED_DECODE(radius * def.steps_per_unit[2]);
            b = FIXED_DECODE(radius * def.steps_per_unit[0]);
            start[0] = FIXED_DECODE(-center[0] * def.steps_per_unit[2]);
            start[1] = FIXED_DECODE(-center[1] * def.steps_per_unit[0]);
            finish[0] = FIXED_DECODE((delta[0] - center[0]) * def.steps_per_unit[2]);
            finish[1] = FIXED_DECODE((delta[1] - center[1]) * def.steps_per_unit[0]);
            if (!def.zx_right)
                cw = !cw;
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
    arc->ready = 1;
}

