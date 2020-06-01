# Description

This is firmware for my stm32 based controller, which executes realtime part of the code for CNC. It supports simplified G-Code commands, and expected to run in pair with control program (https://github.com/vladtcvs/cnccontrol).

# Supported features

## Hardware
- Movements: 3-axis movements X, Y, Z.
- Detectors: XYZ endstops, Probe
- Communication: stm32 serial1 (PA9/PA10), 9600 baud/s, 8N1

## Commands

### Command format

```
[CRC]Nn Gg....
```

[CRC] - 1 byte, sum of other message bytes
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

#### Arc movement
```
G2/G3 XxxYyyZzz G17/G18/G19 Aaaa Baaa Ffff Tttt Pppp Llll
X, Y, Z - relative coordinates, steps. If G17 selected, only X and Y are used, G18 only Y and Z, G19 only Z and X
A, B - axises of ellipse, steps
G17/G18/G19 - selected plane
F, T, P, L - same as for G0/G1
```

Attention: cnccontrol_rt assumes that XYZ are right-handed basis. If it is wrong, you need to exchange G2 and G3 in g-code commands

#### Get/set current state

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

M3 Tttt - enable tool number ttt
M5 - disable tool

This section is for tools that required to start and stop while moving, for example lasers

# Searching endstops and probing

## Searching endstops (simplified)

```
N0 G0 X-300 F50
N1 M998
N2 G0 Y-300 F50
N3 M998
N4 G0 Z-300 F50
N5 M998
N6 M997
```

## Probing

Example sequence for probing. (Axis Z directed to bottom)
```
N0 M996
N1 G0 Z200 F500
N2 M998
N3 G0 Z-1 F100
N4 G0 Z2 F50
N5 M998
N6 M995
```

# Default ports usage

- Serial - PA9/PA10
- X-step - PA0
- X-dir - PA1
- Y-step - PA3
- Y-dir - PA4
- Z-step - PA6
- Z-dir - PA7
- X-endstop - PB14
- Y-endstop-  PB13
- Z-endstop - PB12

- Probe - PA15

# License

GPLv3, see LICENSE file for full text
