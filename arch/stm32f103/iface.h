#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>

void ifaceInitialise(const uint8_t mac[6]);
void ifacePoll(void);
