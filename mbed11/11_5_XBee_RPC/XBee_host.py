import serial
import time

UART = '/dev/ttyACM0'
u = serial.Serial(UART, 9600)

# XBee setting
serdev = '/dev/ttyUSB0'
s = serial.Serial(serdev, 9600)

s.write("+++".encode())
char = s.read(2)
print("Enter AT mode.")
print(char.decode())

s.write("ATMY 132\r\n".encode())
char = s.read(3)
print("Set MY 132.")
print(char.decode())

s.write("ATDL 232\r\n".encode())
char = s.read(3)
print("Set DL 232.")
print(char.decode())

s.write("ATID 0x1\r\n".encode())
char = s.read(3)
print("Set PAN ID 0x1.")
print(char.decode())

s.write("ATWR\r\n".encode())
char = s.read(3)
print("Write config.")
print(char.decode())

s.write("ATMY\r\n".encode())
char = s.read(4)
print("MY :")
print(char.decode())

s.write("ATDL\r\n".encode())
char = s.read(4)
print("DL : ")
print(char.decode())

s.write("ATCN\r\n".encode())
char = s.read(3)
print("Exit AT mode.")
print(char.decode())

print("start sending RPC")
while True:
    # send RPC to remote
    s.write("/AcceVal/run\n\r".encode())
    time.sleep(0.35)
    mbedrtn = u.readline()
    print('Mbed$ ' + str(mbedrtn))

s.close()
