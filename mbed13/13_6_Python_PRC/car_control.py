import time
import serial
import sys,tty,termios

# XBee setting
serdev = '/dev/ttyUSB1'
s = serial.Serial(serdev, 9600)

s.write("+++".encode())
char = s.read(2)
print("Enter AT mode.")
print(char.decode())

s.write("ATMY 234\r\n".encode())
char = s.read(3)
print("Set MY 234.")
print(char.decode())

s.write("ATDL 123\r\n".encode())
char = s.read(3)
print("Set DL 123.")
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

class _Getch:
    def __call__(self):
        fd = sys.stdin.fileno()
        old_settings = termios.tcgetattr(fd)
        try:
            tty.setraw(sys.stdin.fileno())
            ch = sys.stdin.read(1)
        finally:
            termios.tcsetattr(fd, termios.TCSADRAIN, old_settings)
        return ch

def get():
    inkey = _Getch()
    while(1):
        k=inkey()
        if k!='':break
    if k=='\x1b':
        k2 = inkey()
        k3 = inkey()
        if k3=='A':
            print ("up")
            s.write("/goStraight/run 100 \n".encode())
        if k3=='B':
            print ("down")
            s.write("/goStraight/run -100 \n".encode())
        if k3=='C':
            print ("right")
            s.write("/turn/run 100 -0.3 \n".encode())
        if k3=='D':
            print ("left")
            s.write("/turn/run 100 0.3 \n".encode())
        time.sleep(0.5)
        s.write("/stop/run \n".encode())
    elif k=='q':
        print ("quit")
        return 0
    else:
        print ("not an arrow key!")
    return 1

if len(sys.argv) < 1:
    print ("No port input")
# s = serial.Serial(sys.argv[1])
while get():
    i = 0
    time.sleep(0.05)

s.close()