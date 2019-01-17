License GPLv3

This is firmware for my stm32 based controller, which executes realtime part of the code for CNC. It supports simplified G-Code commands, and expected to run in pair with control program (https://github.com/vladtcvs/cnccontrol).

Supported commands:

G0/G1 XxxYyyZzz Ffff Tttt Pppp Llll
X, Y, Z - relative coordinates, mm
F - feed, mm/min
P - initial feed, mm/min
L - finish feed, mm/min
T - acceleration, mm/sec^2

G2/G3 XxxYyyZzz G17/G18/G19 Ddd Ffff Tttt Pppp Llll
X, Y, Z - relative ccordinates, mm. If G17 selected, only X and Y are used, G18 only Y and Z, G19 only Z and X
G17/G18/G19 - selected plane
D - distance from horde to center, mm. Negative, if center is left from horde
F, T, P, L - same as for G0/G1

G28 [X][Y][Z] - find endtops

G31 - Z probe

M114 - current coordinates

M119 - endstops and Z-probe status
