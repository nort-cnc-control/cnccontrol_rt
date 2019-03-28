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
Nn Gg....
```

Line numbering (N) is mandatory.

```
N0 G0 X10 F100
```

### Supported commands
#### Line movement
```
G0/G1 XxxYyyZzz Ffff Tttt Pppp Llll
X, Y, Z - relative coordinates, mm, default=0
F - feed, mm/min, default=5
P - initial feed, mm/min, default=0
L - finish feed, mm/min, default=0
T - acceleration, mm/sec^2, default=50
```

#### Arc movement
```
G2/G3 XxxYyyZzz G17/G18/G19 Ddd Ffff Tttt Pppp Llll
X, Y, Z - relative coordinates, mm. If G17 selected, only X and Y are used, G18 only Y and Z, G19 only Z and X
G17/G18/G19 - selected plane
D - distance from horde to center, mm. Negative, if center is left from horde
F, T, P, L - same as for G0/G1
```

Attention! see config.h and specify XY_RIGHT, YZ_RIGHT, ZX_RIGHT for your CNC. I have axis Z looking down, and XY axes form left pair of vectors, if look from top. This is critical for correct clockwise/counter-clockwise movement

#### Get/set current state

- M114 - current coordinates
- M119 - endstops and Z-probe status
- M995 - disable break on probe
- M996 - enable break on probe
- M997 - set current posiiton to 0, 0, 0 and forget residual delta
- M998 - forget residual delta

#### Reboot

- M999 - reboot

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

# Residual delta

We can make only fixed step with stepper motors. But if we have make such movement, which require non-integer amount of steps, we can not do it precisely. So, some small delta appears. Delta = target position - end of move position. So we add this delta to the next move to prevent accumulation of the error. But when we searching endstops or probing, we have to forget this delta after searching movement.

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

GPLv3+, see LICENSE file for full text
