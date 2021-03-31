#!/bin/sh

openocd -f interface/stlink-v2.cfg -f target/stm32f1x.cfg -c "init; reset halt; flash write_image erase $1 0x08000000 bin; reset; exit"
