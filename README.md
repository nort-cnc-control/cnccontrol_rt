# Description

This is firmware for my stm32 based controller, which executes realtime part of the code for CNC. It supports simplified G-Code commands, and expected to run in pair with control program (https://github.com/nort-cnc-control/NoRTCNCControl).

# Getting sources and building

```
git clone git@github.com:nort-cnc-control/cnccontrol_rt.git
cd cnccontrol_rt
git submodule init
git submodule sync
git submodule update
cd kbuild-standalone
mkdir build
cd build
make -C ../ -f Makefile.sample O=`pwd`
cd kconfig
export PATH=$PATH:`pwd`
cd ../../../
```
Then select configuration by copying config from `arc/defconfig_*` to `.config`, then run

```
make menuconfig
make
```
## Real hardware
and `make flash` if you compile for real hardware (not emulation). avrdude for arduino or ocd for stm32 is required for `make flash`.

## Emulation
```
cd arch/emulation
./controller.elf
```

# Supported features

## Hardware
- Board: stm32f103 'blue pill' and arduino mega2560 + ramps
- Movements: 3-axis movements X, Y, Z.
- Detectors: XYZ endstops, Probe
- Communication: ethernet through enc28j60 (stm32), serial port (arduino)


## RT Commands

### Command format

```
RT: Nn Gg....
```

Line numbering (N) is mandatory.

```
N0 G0 X10 F100
```

### Supported commands
#### Line movement
```
G0/G1 XxxYyyZzz Ffff Tttt Pppp Llll
X, Y, Z - relative coordinates, steps, default=0
F - feed, mm/sec, default=5/60
P - initial feed, mm/sec, default=0
L - finish feed, mm/sec, default=0
T - acceleration, mm/sec^2, default=50
```

#### Helix movement
```
G2/G3 XxxYyyRrrSssHhhDdd G17/G18/G19 Aaaa Baaa Ffff Tttt Pppp Llll
X, Y - end coordinates in system of center of helix, steps.
R, S - start coordinates in system of center of helix, steps.
H - height of helix, steps
D - length of helix, mm
A, B - axises of ellipse, steps
G17/G18/G19 - selected plane
F, T, P, L - same as for G0/G1
```

Attention: cnccontrol_rt assumes that XYZ are right-handed basis. If it is wrong, you need to exchange G2 and G3 in g-code commands

#### Get/set current state

- M3   - start tool
- M5   - stop tool
- M114 - current coordinates
- M119 - endstops and Z-probe status
- M800 - unlock movements
- M801 - lock movements, =True on start
- M802 - disable fail on endstops touch
- M803 - enable fail on endstop touch, =True on start
- M995 - disable break on probe
- M996 - enable break on probe
- M997 - set current posiiton to 0, 0, 0

#### Reboot

- M999 - reboot

#### Using tool

- M3 - enable tool
- M5 - disable tool

This section is for tools that required to start and stop while moving, for example lasers

# Searching endstops and probing

## Searching endstops (simplified)

```
RT: N0 G0 X-30000 F50 T40
RT: N1 M998
RT: N2 G0 Y-30000 F50 T40
RT: N3 M998
RT: N4 G0 Z-30000 F50 T40
RT: N5 M998
RT: N6 M997
```

## Probing

Example sequence for probing. (Axis Z directed to bottom)
```
RT: N0 M996
RT: N1 G0 Z20000 F500 T40
RT: N2 M998
RT: N3 G0 Z-1000 F100 T40
RT: N4 G0 Z2000 F50 T40
RT: N5 M998
RT: N6 M995
```

# Modbus commands

It is also supported modbus master for control spindel, light, cooling, etc.

```
MB:XXXX:YYYY:ZZZZ
```

where XXXX - devic number, YYYY - port number and ZZZZ - value

# Default ports usage for stm32f103 blue pill

- X-step - PC14
- X-dir - PC15
- Y-step - PA0
- Y-dir - PA1
- Z-step - PA2
- Z-dir - PA3
- X-endstop - PB7
- Y-endstop-  PB6
- Z-endstop - PB5

- Probe - Pb8
- Tool - PB1

# License

GPLv3, see LICENSE file for full text
