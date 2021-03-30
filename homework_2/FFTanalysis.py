import matplotlib.pyplot as plt
import numpy as np
import serial
import time

Fs = 1000.0;            # sampling rate(Hz)
Ts = 1.0/Fs;            # sampling period(T)
t = np.arange(0,1,Ts)   # time vector; create Fs samples between 0 and 1.0 sec.
y = np.arange(0,1,Ts)   # signal vector; create Fs samples

n = len(y)              # length of the signal
k = np.arange(n)        # k = [0, 1, 2, ... , n]     
T = n/Fs                # Total Interval
frq = k/T               # a vector of frequencies; two sides frequency range
frq = frq[range(int(n/2))]      # one side frequency range

serdev = '/dev/ttyACM0'
s = serial.Serial(serdev)
for x in range(0, int(Fs)):
    line=s.readline() # Read an echo string from B_L4S5I_IOT01A terminated with '\n'
    # print line
    y[x] = float(line)

sin = 0.5*np.sin(2*np.pi*50*t)+0.5
SIN = np.fft.fft(sin)/n*2
SIN = SIN[range(int(n/2))]
Y = np.fft.fft(y)/n*2           # fft computing and normalization
Y = Y[range(int(n/2))]          # remove the conjugate frequency parts


fig, ax = plt.subplots(2, 1)

ax[0].plot(t,y)
ax[0].plot(t, sin)
ax[0].set_xlabel('Time')
plt.ylim(0, 1)
ax[0].set_ylabel('Amplitude')

ax[1].plot(frq,abs(Y),'r') # plotting the spectrum
ax[1].plot(frq,abs(SIN), 'k')
ax[1].set_xlabel('Freq (Hz)')
ax[1].set_ylabel('|Y(freq)|')

plt.show()
s.close()