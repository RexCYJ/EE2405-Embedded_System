Class:      EE2405, Embedded System Lab.
Date:       Apr. 7, 2021. 
Filename:   Exam 1.
Author:     108061121,  Yu-Jia, Chen.

Content:    Readme.txt
            main.cpp
            4dgl-ulcd-se/
            FFT.py
            <some pictures for demo>

Connect:    uLCD(RX: D0, TX: D1, RES: D2);
            Vout(DAC): D7; 
            Vin(ADC): D6;
            Button(Up: D3, Down: D4, Confirm: D5);

Setup & Run:
    1) Connect the device as the description above correctly.
    2) Execute main.cpp. Type the instruction below in your terminal
        $ sudo mbed compile --source . --source ~/ee2405/mbed-os-build/ -m B_L4S5I_IOT01A -t GCC_ARM -f
    3) Press Up/Down button to move the cursor on uLCD and select the ideal rate.
       As soon as the cursor points to the rate, it would generate the corresponding wave from DAC.
    4) Go to your terminal and operate FFT.py. By type the instruction below:
        $ sudo python3 FFT.py
    5) Press Confirm button on your bread board, and the STM32 would start to record the ADC and transmit received data after a second.
    6) The python program receives the data, and show the wave with FFT result on the plot.

Source file:
    1) mbed.h
    2) uLCD_4DGL.h

Function description
    1) void Main_init() :
        Initialize the program.
    2) void Button_up() :
        Interrupt function called when button up pressed. It changes the wave form and put MenuCursor() into EventQueue
    3) void Button_down() :
        Interrupt function called when button down pressed. It changes the wave form and put MenuCursor() into EventQueue
    4) void Button_confirm() :
        Push Waveread() and WaveTrans() into Eventqueue, and also push SamplingIndicator() and MenuCursor to change the cursor
    5) void Wavegen() :
        Every run of it generate a complete wave.
    6) void Waveread() :
        Read the AnalogIn Vin received data every 1ms within 1s.
    7) void Wavetrans() :
        Transmit the received data to PC after Waveread() finishs.
    8) void MenuCursor() :
        Clear the current cursor position on the uLCD, and set the cursor at the next position
    9) void SamplingIndicator() :
        Change the cursor to '@', which indicate the system is reading the data and transmitting the data.

Result: 
    The waves are identical to what we observed by picoscope. 
    For more details, please see the pictures.
