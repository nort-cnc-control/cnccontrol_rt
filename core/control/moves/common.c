#include <stdint.h>
#include <math.h>

#define SQR(a) ((a) * (a))

// Find delay between ticks
//
// feed in mm / min
// len is measured in mm
//
// Return: delay in usec
uint32_t feed2delay(double feed, double step_len)
{
    if (feed < 1)
        feed = 1;
    return step_len * 1e6 / (feed / 60);
}

// Find new feed when acceleration
//
// feed in mm / min
// acc in mm / sec^2
// delay in usec
//
// Return: new feed in mm / min
double accelerate(double feed, double acc, double delay)
{
    acc *= 3600; // mm / min^2
    delay /= (60 * 1e6); // min

    double df = acc * delay;
    return feed + df;
}

// Find amount of acceleration steps from feed0 to feed1
//
// feed0 in mm / min
// feed1 in mm / min
// acc in mm / sec^2
// len in mm
uint32_t acceleration_steps(double feed0,
                            double feed1,
                            double acc,
                            double len,
                            uint32_t steps)
{
    double cacc = acc * 3600;
    // cacc in mm / min^2
    double slen = (SQR(feed1) - SQR(feed0)) / (2*cacc);
    // slen in mm
    return slen * steps / len;
}
