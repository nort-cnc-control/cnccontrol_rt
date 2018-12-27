#include <fixed.h>
#include <math.h>

// Find delay between ticks
//
// feed in mm / min
// len is measured in mm
//
// Return: delay in usec
uint32_t feed2delay(fixed feed, fixed len, uint32_t steps)
{
    if (feed == 0)
        feed = 1;

    return FIXED_DECODE(DIV(len * 60 * 1000000, feed) / steps);
}

// Find new feed when acceleration
//
// feed in 0.001 mm / min
// acc in mm / sec^2
// delay in usec
//
// Return: new feed in 0.001 mm / min
fixed accelerate(fixed feed, int32_t acc, int32_t delay)
{
    //feed + ( FIXED_ENCODE(acc) * 60*60) * (delay / (60 * 1000000UL));
    int df = FIXED_ENCODE(acc) * 60 * delay / 1000000;
    if (df == 0 && acc > 0)
        df = 1;
    else if (df == 0 && acc < 0)
        df = -1;
    return feed + df;
}

// Find amount of acceleration steps from feed0 to feed1
//
// feed0 in 0.001 mm / min
// feed1 in 0.001 mm / min
// acc in mm / sec^2
// len in 0.001 mm
uint32_t acceleration_steps(fixed feed0,
                            fixed feed1,
                            int32_t acc,
                            fixed len,
                            uint32_t steps)
{
    int64_t cacc = acc;
    cacc = FIXED_ENCODE(cacc * 3600);
    // cacc in 0.001 mm / min^2
    fixed slen = DIV(SQR(feed1) - SQR(feed0), 2*cacc);
    // len in 0.001 mm
    return FIXED_DECODE(DIV(slen * steps, len));
}

