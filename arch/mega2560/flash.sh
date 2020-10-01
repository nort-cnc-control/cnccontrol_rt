#avrdude -c wiring -p atmega2560 -U flash:w:controller.bin -P /dev/ttyACM0 -B 115200
#avrdude -c usbasp -p atmega2560 -U flash:w:controller.bin 
avrdude -v -patmega2560 -cwiring -P/dev/ttyACM0 -b115200 -D -Uflash:w:controller.bin

