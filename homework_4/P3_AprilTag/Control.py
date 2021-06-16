import time
import serial

# XBee setting
serdev = '/dev/ttyUSB0'
s = serial.Serial(serdev, 9600)

print("start sending RPC")

USBPORT = False

if USBPORT:
    mbeddev = '/dev/ttyACM0'
    mbed = serial.Serial(mbeddev, 9600)

while True:
    cmdline = input("Input: ")
    s.write("/apriltag/run\n".encode())
    while USBPORT:
        line = mbed.readline()
        print(line)