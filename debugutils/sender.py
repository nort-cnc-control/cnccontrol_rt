#!/usr/bin/env python3

import time
import getopt
import sys
import serial
import re

baudrate = 57600
port = "/dev/ttyUSB0"
f = None

def usage():
    print("Usage: sender.py [-h] [-b baudrate] [-p serial port] -f gcode_file")

try:
    optlist, args = getopt.getopt(sys.argv[1:], "hf:p:b:")
except getopt.GetoptError as err:
    usage()
    sys.exit(1)

for o, a in optlist:
    if o == "-h":
        usage()
        sys.exit(0)
    elif o == "-f":
        f = a
    elif o == "-b":
        baudrate = int(a)
    elif o == "-p":
        port = a
    else:
        usage()
        sys.exit(1)
if f == None:
    usage()
    sys.exit(1)

ser = serial.Serial(port, baudrate, bytesize=8, parity='N', stopbits=1, timeout=1)

source = open(f, "r")
gcoderaw = source.readlines()
source.close()

gcode = []

for gc in gcoderaw:
    gc = gc.strip("\n")
    gc = gc.strip("\r")
    if len(gc) > 0:
        gcode.append(gc)

for gc in gcode:
    print("Sending: %s" % gc)
    cmd = gc + "\n"
    ser.write(bytes(cmd, "UTF-8"))
    ser.flush()
    ans = ""
    while ans == "":
        ans = ser.readline().decode("UTF-8")
    print("Ans: %s" % ans)
    #time.sleep(0.5)

ser.close()

