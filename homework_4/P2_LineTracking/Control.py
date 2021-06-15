import time
import serial

# XBee setting
serdev = '/dev/ttyUSB0'
s = serial.Serial(serdev, 9600)
# s.write("+++".encode())
# char = s.read(2)
# print("Enter AT mode.")
# print(char.decode())
# s.write("ATMY 123\r\n".encode())
# char = s.read(3)
# print("Set MY 123.")
# print(char.decode())
# s.write("ATDL 456\r\n".encode())
# char = s.read(3)
# print("Set DL 456.")
# print(char.decode())
# s.write("ATID 0x1\r\n".encode())
# char = s.read(3)
# print("Set PAN ID 0x1.")
# print(char.decode())
# s.write("ATWR\r\n".encode())
# char = s.read(3)
# print("Write config.")
# print(char.decode())
# s.write("ATMY\r\n".encode())
# char = s.read(4)
# print("MY :")
# print(char.decode())
# s.write("ATDL\r\n".encode())
# char = s.read(4)
# print("DL : ")
# print(char.decode())
# s.write("ATCN\r\n".encode())
# char = s.read(3)
# print("Exit AT mode.")
# print(char.decode())

print("start sending RPC")

USBPORT = False
if USBPORT:
    mbeddev = '/dev/ttyACM0'
    mbed = serial.Serial(mbeddev, 9600)

while True:
    cmdline = input("Input: ")
    s.write("/track/run\n".encode())
    while USBPORT:
        line = mbed.readline()
        print(line)
    # time.sleep(5)