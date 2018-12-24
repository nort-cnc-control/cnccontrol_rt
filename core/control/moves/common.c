#include <fixed.h>

// Find delay between ticks
//
// feed in 0.001 mm / min
// len is measured in 0.001 mm
//
// Return: delay in usec
uint32_t feed2delay(fixed feed, fixed len, uint32_t steps)
{
	if (feed == 0)
		feed = 1;

	return len * 60UL * 1000000UL / feed / steps;
}

// Find new feed when acceleration
//
// feed in 0.001 mm / min
// acc in mm / sec^2
// delay in usec
//
// Return: new feed in 0.001 mm / min
fixed accelerate(fixed feed, int32_t acc, uint32_t delay)
{
	//feed + ( FIXED_ENCODE(acc) * 60*60) * (delay / (60 * 1000000UL));
    return feed + FIXED_ENCODE(acc) * 60 * delay / 1000000UL;
}

// Find amount of acceleration steps from feed0 to feed1
//
// feed0 in 0.001 mm / min
// feed1 in 0.001 mm / min
// acc in mm / sec^2
// len in 0.001 mm
uint32_t acceleration_steps(fixed feed0,
                            fixed feed1,
                            uint32_t acc,
                            fixed len,
                            uint32_t steps)
{
	uint32_t s = 0;
	fixed fe = feed0;
	while (fe < feed1)
	{
		uint32_t delay = feed2delay(fe, len, steps);
		fe = accelerate(fe, acc, delay);
		s++;
	}
	return s;
}
