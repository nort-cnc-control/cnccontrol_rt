#include <gcode/gcodes.h>
#include <assert.h>
#include <stdio.h>

void test_G0(void)
{
    gcode_frame_t frame;
    const char gcode[] = "G0X10Y10F100P0L0";
    int res = parse_cmdline(gcode, &frame);
    assert(res == -E_OK);
    assert(frame.num == 6);
}

void test_G28(void)
{
    gcode_frame_t frame;
    const char gcode[] = "G0X";
    int res = parse_cmdline(gcode, &frame);
    assert(res == -E_OK);
    assert(frame.num == 2);
    assert(frame.cmds[1].type == 'X');
    assert(frame.cmds[1].val_i == 0);
}

int main(void)
{
    test_G0();
    test_G28();
    return 0;
}
