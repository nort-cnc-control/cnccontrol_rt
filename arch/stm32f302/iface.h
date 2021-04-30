#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>

uint8_t spi_rw(uint8_t data);
void spi_cs(bool enable);
void spi_write_buf(const uint8_t *data, size_t len);
void spi_read_buf(uint8_t *data, size_t len);

void spi_setup(void);

void ifaceInitialise(const uint8_t mac[6]);
void ifaceStart(void);
