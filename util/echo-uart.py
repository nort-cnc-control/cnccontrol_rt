import serial
import sys

N    = 60
baud = 38400
port = sys.argv[1]

ser = serial.Serial(port, baud)

ser.write(b'START:\n')
rx = ser.readline()
print('RX: ', rx.strip())


for i in range(N):
	msg = "ECHO: " + "".join(["**"]*i) + "\n"
	tx = bytes(msg, 'utf8')
	print('TX: ', tx.strip())
	ser.write(tx)
	rx = ser.readline()
	print('RX: ', rx.strip())

ser.close()

