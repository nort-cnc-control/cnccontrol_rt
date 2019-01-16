#include <stdint.h>
#include <math.h>

#define SQR(a) ((a) * (a))

// Find delay between ticks
//
// feed in mm / min
// len is measured in mm
//
// Return: delay in usec
uint32_t feed2delay(double feed, double len, uint32_t steps)
{
    if (feed == 0)
        feed = 1;

    return len * 60 * 1000000 / feed / steps;
}

// Find new feed when acceleration
//
// feed in 0.001 mm / min
// acc in mm / sec^2
// delay in usec
//
// Return: new feed in 0.001 mm / min
double accelerate(double feed, int32_t acc, int32_t delay)
{
    int df = acc * 60 * delay / 1000000;
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
uint32_t acceleration_steps(double feed0,
                            double feed1,
                            int32_t acc,
                            double len,
                            uint32_t steps)
{
    int64_t cacc = acc;
    cacc = cacc * 3600;
    // cacc in 0.001 mm / min^2
    double slen = SQR(feed1) - SQR(feed0) / 2*cacc;
    // len in 0.001 mm
    return slen * steps / len;
}

