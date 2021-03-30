Class:      EE2405, Embedded System Lab.
Date:       Mar. 30, 2021. 
Filename:   Homework 2.
Author:     108061121,  Yu-Jia, Chen.

Content:    Readme.txt
            main.cpp
            FFTanalysis.py
            <some demo pictures>

Connect:    uLCD(RX: D0, TX: D1, RES: D2);
            Vout(DAC): D7; Vin(ADC): D6;
            Button(Up: D3, Down: D4, Confirm: D5);

Low Pass Filter:

                  R = 14.1kΩ
    Vout >——————/VVV\———┬——————————> Vin
                        |
                      ——┴——  C = 141.9nF
                      ——┬——
                        |
                        〨

Setup & Run:
    1) Connect the device as above correctly.
    2) Execute main.cpp.
    3) Press Up/Down button to move the cursor on uLCD and select the ideal frequency.
    4) Switch to your terminal and operate FFTanalysis.py.
    5) Press Confirm button, and the STM32 would start to record the ADC and transmit received data after a second.
    6) The python program receives the data, and show the wave with FFT result on the plot.

Result: 
    The waves are transformed after passing through the low-pass filter. With its frequency increasing, the corners of the waves are compressed as curve, and, thus, become less similar to the original triangle wave.
    For more details, please see the pictures.

