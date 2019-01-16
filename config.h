#pragma once

#define SIZE_X 224.0
#define SIZE_Y 324.0
#define SIZE_Z 105.0

#define FULL_STEPS 200
#define MICRO_STEP 8
#define MM_PER_ROUND 4.0

#define FEED_MAX 1500
#define FEED_ES_TRAVEL 800
#define FEED_ES_PRECISE 30
#define FEED_PROBE_TRAVEL 200
#define FEED_PROBE_PRECISE 20
#define FEED_BASE 5
#define ACC 60

#define STEPS_PER_ROUND ((FULL_STEPS) * (MICRO_STEP))
#define STEPS_PER_MM ((STEPS_PER_ROUND) / (MM_PER_ROUND))

#define SHELL_BAUDRATE 57600

#define XY -1
#define YZ 1
#define ZX 1
