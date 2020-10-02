#define __STDC_WANT_DEC_FP__
#include <math.h>
#include <stdlib.h>
#include <assert.h>

#include <control/moves/moves_common/common.h>
#include <control/moves/moves_common/acceleration.h>
#include <control/moves/moves_arc/arc.h>


#define SQR(a) ((a) * (a))
const double pi = 3.1415926535;

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
    return round(mx);
}

static double fun2(int32_t x, int32_t a2, int32_t b2)
{
    double fx = x;
    return b2 * (1 - fx*fx / a2);
}

// Running

static arc_plan *current_plan;

typedef struct {
    double a2;
    double b2;
    int32_t position_x;
    int32_t position_y;
    int dx;
    arc_segment_id segment;
    int32_t x1;
    int32_t y1;
    int32_t x2;
    int32_t y2;
} arc_segment_state;

typedef struct {
    arc_plane plane;
    double a2;
    double b2;
    int32_t position_x;
    int32_t position_y;
} arc_plane_state;


static struct
{
    int segment_id;
    int32_t x0;
    int32_t y0;
    int32_t x1;
    int32_t y1;
    int32_t a;
    int32_t b;
    int dx;
    int x;
    int y;
    double step_str;
    double step_hyp;

    arc_segment_state segment;
    arc_plane_state plane;

    struct {
        int32_t position[3];
    } global;

    int32_t position[3];
    int32_t dir[3];
    acceleration_state acc;
} current_state;

static void global_update_position(void)
{
    switch (current_state.plane.plane)
    {
        case XY: // XY
            current_state.global.position[0] = current_state.plane.position_x;
            current_state.global.position[1] = current_state.plane.position_y;
            break;

        case YZ: // YZ
            current_state.global.position[1] = current_state.plane.position_x;
            current_state.global.position[2] = current_state.plane.position_y;
            break;

        case ZX: // ZX
            current_state.global.position[2] = current_state.plane.position_x;
            current_state.global.position[0] = current_state.plane.position_y;
            break;
    }
}

static void plane_update_position(void)
{
    int32_t x, y;

    switch (current_state.segment.segment)
    {
        case RIGHT: // right
            x = current_state.segment.position_y;
            y = -current_state.segment.position_x;
            break;

        case TOP: // top
            x = current_state.segment.position_x;
            y = current_state.segment.position_y;
            break;

        case LEFT: // left
            x = -current_state.segment.position_y;
            y = current_state.segment.position_x;
            break;

        case BOTTOM: // bottom
            x = -current_state.segment.position_x;
            y = -current_state.segment.position_y;
            break;

        default:
            // error
            break;
    }

    current_state.plane.position_x = x;
    current_state.plane.position_y = y;

    global_update_position();
}

static void segment_make_step_x(int delta)
{
    current_state.segment.position_x += delta;
    plane_update_position();
}

static void segment_make_step_y(int delta)
{
    current_state.segment.position_y += delta;
    plane_update_position();
}

static bool segment_tick(void)
{
    if (current_state.segment.position_x == current_state.segment.x2)
    {
        // end of the segment
        return false;
    }
    segment_make_step_x(current_state.segment.dx);

    double y2 = fun2(current_state.segment.position_x, current_state.segment.a2, current_state.segment.b2);
    if (fabs(SQR(current_state.segment.position_y - 1) - y2) < fabs(SQR(current_state.segment.position_y) - y2))
    {
        segment_make_step_y(-1);
    }
    else if (fabs(SQR(current_state.segment.position_y + 1) - y2) < fabs(SQR(current_state.segment.position_y) - y2))
    {
        segment_make_step_y(1);
    }

    return true;
}

static void load_segment(int id)
{
    current_state.segment.segment = current_plan->geometry.segments[id].segment;
   
    double x1 = current_plan->geometry.segments[id].x1;
    double y1 = current_plan->geometry.segments[id].y1;
    double x2 = current_plan->geometry.segments[id].x2;
    double y2 = current_plan->geometry.segments[id].y2;
    
    double lx1, lx2, ly1, ly2;
    switch (current_state.segment.segment)
    {
        case RIGHT: // right
            lx1 = -y1;
            ly1 = x1;
            lx2 = -y2;
            ly2 = x2;
            break;
        case TOP: // top
            lx1 = x1;
            ly1 = y1;
            lx2 = x2;
            ly2 = y2;
            break;
        case LEFT: // left
            lx1 = y1;
            ly1 = -x1;
            lx2 = y2;
            ly2 = -x2;
            break;
        case BOTTOM: // bottom
            lx1 = -x1;
            ly1 = -y1;
            lx2 = -x2;
            ly2 = -y2;
            break;
        default:
            // error
            break;
    }

    current_state.segment.x1 = lx1;
    current_state.segment.y1 = ly1;

    current_state.segment.x2 = lx2;
    current_state.segment.y2 = ly2;

    current_state.segment.position_x = current_state.segment.x1;
    current_state.segment.position_y = current_state.segment.y1;

    if (current_state.segment.segment == 1 || current_state.segment.segment == 3)
    {
        current_state.segment.a2 = current_state.plane.b2;
        current_state.segment.b2 = current_state.plane.a2;
    }
    else
    {
        current_state.segment.a2 = current_state.plane.a2;
        current_state.segment.b2 = current_state.plane.b2;
    }

    if (current_state.segment.x1 < current_state.segment.x2)
    {
        current_state.segment.dx = 1;
    }
    else
    {
        current_state.segment.dx = -1;
    }
}

static bool load_next_segment(void)
{
    if (current_state.segment_id == current_plan->geometry.num_segments - 1)
        return false;

    current_state.segment_id += 1;
    load_segment(current_state.segment_id);
    return true;
}

static bool load_first_segment(void)
{
    int i;
    if (current_plan->geometry.num_segments == 0)
        return false;
    current_state.segment_id = 0;
    for (i = 0; i < 3; i++)
    {
        current_state.position[i] = position.pos[i];
    }
    load_segment(current_state.segment_id);
    plane_update_position();
    return true;
}

static bool make_tick(void)
{
    int i;
    int32_t pos[3];
    for (i = 0; i < 3; i++)
        pos[i] = current_state.global.position[i];

    while (segment_tick() == false)
    {
        if (load_next_segment() == false)
            return false;
    }

    // make steps
    for (i = 0; i < 3; i++)
    {
        int d = current_state.global.position[i] - pos[i];
        if (d != 0)
        {
            moves_common_set_dir(i, d >= 0);
            moves_common_make_step(i);
        }

        current_state.dir[i] = d;
        current_state.position[i] += d;
    }

    // Set current position
//    moves_common_set_position(current_state.position);

    return true;
}


// module API functions

int32_t arc_step_tick(void)
{
    // Check for endstops
    if (current_plan->check_break && current_plan->check_break(current_state.dir, current_plan->check_break_data))
    {
        moves_common_endstops_touched();
        return -E_ENDSTOP;
    }

    // Make step
    if (!make_tick())
    {
        moves_common_line_finished();
        return -E_NEXT;
    }

    // Calculate delay

    double step_len = current_plan->len / current_plan->steps;
    double step_delay = step_len / current_state.acc.feed;
//    printf("%lf\n", current_state.acc.feed);
    acceleration_process(&current_state.acc, step_delay);
    return step_delay * 1000000L;
}

int arc_move_to(arc_plan *plan)
{
    current_plan = plan;
    if (!plan->ready)
    {
        arc_pre_calculate(plan);
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

    current_state.plane.plane = current_plan->plane;
    current_state.plane.a2 = current_plan->geometry.a2;
    current_state.plane.b2 = current_plan->geometry.b2;
   
    if (!load_first_segment())
    {
        return -E_NEXT;
    }
 
    moves_common_line_started();
    return -E_OK;
}

// Pre-calculating
static arc_segment_id find_segment(int x, int y, int32_t xg, int32_t yg)
{
    if (x >= xg)
        return RIGHT;
    else if (x <= -xg)
        return LEFT;
    else if (y >= yg)
        return TOP;
    else if (y <= -yg)
        return BOTTOM;
    else
        return -1; // error
}

static arc_segment_id next_segment(arc_segment_id s)
{
    switch (s)
    {
    case RIGHT:
        return TOP;
    case TOP:
        return LEFT;
    case LEFT:
        return BOTTOM;
    case BOTTOM:
        return RIGHT;
    }
}

static arc_segment_id prev_segment(arc_segment_id s)
{
    switch (s)
    {
    case RIGHT:
        return BOTTOM;
    case BOTTOM:
        return LEFT;
    case LEFT:
        return TOP;
    case TOP:
        return RIGHT;
    }
}

static double normalize_angle(double angle, arc_segment_id s)
{
    double center;
    switch (s)
    {
    case RIGHT:
        center = 0;
        break;
    case TOP:
        center = M_PI / 2;
        break;
    case LEFT:
        center = M_PI;
        break;
    case BOTTOM:
        center = M_PI * 3/2;
        break;
    }
    while (angle < center - M_PI)
        angle += 2 * M_PI;
    while (angle > center + M_PI)
        angle -= 2 * M_PI;
    return angle;
}

int32_t x1segment(arc_segment_id s, int32_t xg)
{
    if (s == RIGHT || s == TOP)
        return xg;
    return -xg;
}

int32_t x2segment(arc_segment_id s, int32_t xg)
{
    if (s == RIGHT || s == BOTTOM)
        return xg;
    return -xg;
}

int32_t y1segment(arc_segment_id s, int32_t yg)
{
    if (s == LEFT || s == TOP)
        return yg;
    return -yg;
}

int32_t y2segment(arc_segment_id s, int32_t yg)
{
    if (s == RIGHT || s == TOP)
        return yg;
    return -yg;
}


void arc_pre_calculate(arc_plan *arc)
{
    int32_t delta[2];
    double stpu_x;
    double stpu_y;
    
    switch (arc->plane)
    {
    case XY:
        delta[0] = arc->x[0];
        delta[1] = arc->x[1];
        stpu_x = moves_common_def->steps_per_unit[0];
        stpu_y = moves_common_def->steps_per_unit[1];
        break;
    case YZ:
        delta[0] = arc->x[1];
        delta[1] = arc->x[2];
        stpu_x = moves_common_def->steps_per_unit[1];
        stpu_y = moves_common_def->steps_per_unit[2];
        break;
    case ZX:
        delta[0] = arc->x[2];
        delta[1] = arc->x[0];
        stpu_x = moves_common_def->steps_per_unit[2];
        stpu_y = moves_common_def->steps_per_unit[0];
        break;
    }

    double a = fabs(arc->a);
    double b = fabs(arc->b);

    bool small_arc = (arc->a >= 0);

    double a2 = a*a;
    double b2 = b*b;

    double k = sqrt(delta[0]*delta[0] / a2 + delta[1]*delta[1] / b2);
    double d = atan2(delta[1]/b, delta[0]/a);

    double g1 = d - M_PI + acos(k/2);
    double x11 = a * cos(g1);
    double y11 = b * sin(g1);

    double g2 = d - M_PI - acos(k/2);
    double x12 = a * cos(g2);
    double y12 = b * sin(g2);

    double x1d;
    double y1d;

    int cw = arc->cw;
    if (small_arc)
    {
        if (!cw)
        {
            if (x11*delta[1] - y11*delta[0] >= 0)
            {
                x1d = x11;
                y1d = y11;
            }
            else
            {
                x1d = x12;
                y1d = y12;
            }
        }
        else
        {
            if (x11*delta[1] - y11*delta[0] >= 0)
            {
                x1d = x12;
                y1d = y12;
            }
            else
            {
                x1d = x11;
                y1d = y11;
            }           
        }
    }
    else
    {
        if (cw)
        {
            if (x11*delta[1] - y11*delta[0] >= 0)
            {
                x1d = x11;
                y1d = y11;
            }
            else
            {
                x1d = x12;
                y1d = y12;
            }
        }
        else
        {
            if (x11*delta[1] - y11*delta[0] >= 0)
            {
                x1d = x12;
                y1d = y12;
            }
            else
            {
                x1d = x11;
                y1d = y11;
            }           
        }
    }

    // round start and end points
    int32_t x1 = round(x1d);
    int32_t y1 = round(y1d);
    int32_t x2 = x1 + delta[0];
    int32_t y2 = y1 + delta[1];

    // correct a2 and b2

    if (y1*y1 != y2*y2)
    {
        a2 = ((double)(y1*y1*x2*x2 - y2*y2*x1*x1)) / (y1*y1-y2*y2);
    }

    if (x1*x1 != x2*x2)
    {
        b2 = ((double)(x1*x1*y2*y2 - x2*x2*y1*y1)) / (x1*x1-x2*x2);
    }

    arc->geometry.a2 = a2;
    arc->geometry.b2 = b2;

    // find points where |dy/dx| == 1
    double xc = a2 / sqrt(a2 + b2);
    double yc = b2 / sqrt(a2 + b2);

    int32_t xg = round(xc);
    int32_t yg = round(sqrt(fun2(xg, a2, b2)));

#if DEBUG
    if (fabs(x1) == xg)
        assert(fabs(y1) == yg);
    if (fabs(x2) == xg)
        assert(fabs(y2) == yg);
#endif

    arc_segment_id s1 = find_segment(x1, y1, xg, yg);
    arc_segment_id s2 = find_segment(x2, y2, xg, yg);

    arc_segment_id s = s1;
    arc_segment_id (*next_f)(arc_segment_id);
    int32_t (*xb_f)(arc_segment_id s, int32_t xg);
    int32_t (*xe_f)(arc_segment_id s, int32_t xg);
    int32_t (*yb_f)(arc_segment_id s, int32_t yg);
    int32_t (*ye_f)(arc_segment_id s, int32_t yg);

    if (!cw)
    {
        next_f = next_segment;
        xb_f = x1segment;
        xe_f = x2segment;
        yb_f = y1segment;
        ye_f = y2segment;
    }
    else
    {
        next_f = prev_segment;
        xb_f = x2segment;
        xe_f = x1segment;
        yb_f = y2segment;
        ye_f = y1segment;
    }

    int32_t steps = 0;
    double len = 0;

    if (s1 != s2)
    {       
        bool end = false;
        arc->geometry.num_segments = 0;
        while (!end)
        {
            int32_t xs1, ys1;
            int32_t xs2, ys2;
            if (s == s1)
            {
                xs1 = x1;
                ys1 = y1;
                xs2 = xe_f(s, xg);
                ys2 = ye_f(s, yg);
            }                
            else if (s == s2)
            {
                xs1 = xb_f(s, xg);
                ys1 = yb_f(s, yg);
                xs2 = x2;
                ys2 = y2;
                end = true;
            }
            else
            {
                xs1 = xb_f(s, xg);
                ys1 = yb_f(s, yg);
                xs2 = xe_f(s, xg);
                ys2 = ye_f(s, yg);
            }

            arc->geometry.segments[arc->geometry.num_segments].segment = s;
            arc->geometry.segments[arc->geometry.num_segments].x1 = xs1;
            arc->geometry.segments[arc->geometry.num_segments].y1 = ys1;
            arc->geometry.segments[arc->geometry.num_segments].x2 = xs2;
            arc->geometry.segments[arc->geometry.num_segments].y2 = ys2;
            switch (s)
            {
            case LEFT:
            case RIGHT:
                steps += abs(ys2 - ys1);
                len += abs(ys2 - ys1) / stpu_y;
                break;
            case TOP:
            case BOTTOM:
                steps += abs(xs2 - xs1);
                len += abs(xs2 - xs1) / stpu_x;
                break;
            }
            arc->geometry.num_segments += 1;
            s = next_f(s);
        }
    }
    else
    {
        double ang1 = normalize_angle(atan2(y1, x1), s);
        double ang2 = normalize_angle(atan2(y2, x2), s);
        if (!cw && ang2 > ang1 || cw && ang2 < ang1)
        {
            // single segment
            arc->geometry.num_segments = 1;
            arc->geometry.segments[0].segment = s;
            arc->geometry.segments[0].x1 = x1;
            arc->geometry.segments[0].y1 = y1;
            arc->geometry.segments[0].x2 = x2;
            arc->geometry.segments[0].y2 = y2; 
            switch (s)
            {
            case LEFT:
            case RIGHT:
                steps += abs(y2 - y1);
                len += abs(y2 - y1) / stpu_y;
                break;
            case TOP:
            case BOTTOM:
                steps += abs(x2 - x1);
                len += abs(x2 - x1) / stpu_x;
                break;
            }
        }
        else
        {
            // 5 segments
            arc->geometry.num_segments = 5;
            int32_t xs1, ys1;
            int32_t xs2, ys2;

            xs1 = x1;
            ys1 = y1;
            xs2 = xe_f(s, xg);
            ys2 = ye_f(s, yg);

            arc->geometry.segments[0].segment = s;
            arc->geometry.segments[0].x1 = xs1;
            arc->geometry.segments[0].y1 = ys1;
            arc->geometry.segments[0].x2 = xs2;
            arc->geometry.segments[0].y2 = ys2;

            switch (s)
            {
            case LEFT:
            case RIGHT:
                steps += abs(ys2 - ys1);
                len += abs(ys2 - ys1) / stpu_y;
                break;
            case TOP:
            case BOTTOM:
                steps += abs(xs2 - xs1);
                len += abs(xs2 - xs1) / stpu_x;
                break;
            }


            s = next_f(s);
            int i;
            for (i = 0; i < 4; i++)
            {
                xs1 = xb_f(s, xg);
                ys1 = yb_f(s, yg);
                xs2 = xe_f(s, xg);
                ys2 = ye_f(s, yg);

                arc->geometry.segments[i].segment = s;
                arc->geometry.segments[i].x1 = xs1;
                arc->geometry.segments[i].y1 = ys1;
                arc->geometry.segments[i].x2 = xs2;
                arc->geometry.segments[i].y2 = ys2;

                s = next_f(s);
            }

            xs1 = xb_f(s, xg);
            ys1 = yb_f(s, yg);
            xs2 = x2;
            ys2 = y2;

            arc->geometry.segments[4].segment = s;
            arc->geometry.segments[4].x1 = xs1;
            arc->geometry.segments[4].y1 = ys1;
            arc->geometry.segments[4].x2 = xs2;
            arc->geometry.segments[4].y2 = ys2;
        }
    }

    arc->steps = steps;

    arc->len = len;
    arc->acc_steps = acceleration_steps(arc->feed0, arc->feed, arc->acceleration, arc->len / arc->steps);
    arc->dec_steps = acceleration_steps(arc->feed1, arc->feed, arc->acceleration, arc->len / arc->steps);
    if (arc->acc_steps + arc->dec_steps > arc->steps)
    {
        int32_t ds = (arc->acc_steps + arc->dec_steps - arc->steps) / 2;
        arc->acc_steps -= ds;
        arc->dec_steps -= ds;
        if (arc->acc_steps + arc->dec_steps < arc->steps)
            arc->acc_steps += (arc->steps - arc->acc_steps - arc->dec_steps);
    }
    if (arc->acc_steps < 0 || arc->dec_steps < 0)
    {
        // We can not perform such moving!
        if (arc->acc_steps < 0)
            arc->acc_steps = 0;
        if (arc->dec_steps < 0)
            arc->dec_steps = 0;
                
    }
    arc->ready = 1;
}

