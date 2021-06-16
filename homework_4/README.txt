Class:      EE2405, Embedded System Lab.
Date:       June. 16, 2021. 
Filename:   Homework 4.
Author:     108061121,  Yu-Jia, Chen.

Content:    Readme.txt
            P1_SelfParking
            P2_LineTracking
            P3_AprilTag

Connect:    OpenMV  tx: D1,     rx: D0
            Xbee    tx: D10,    rx: D9
            pcUSB   tx: USBTX,  rx: USBRX
            Right servo: D6, Left servo: D5

Description:
    P1_SelfParking:
        Use Xbee to communicate with the bbcar and send the position 
        data of the car to the mbed, the mbed system would calculate 
        the corresponsing distance and the angle, and then lead itself 
        back to the destination.
        The self-parking consists of four parts. First, the car corrects 
        its angle, and then walks to the front of destination. Later, 
        rotate itself with 90 degree. Finally, goes in to the parking 
        place.
    
    P2_LineTracking:
        The car use OpenMV to detect the line. OpenMV use internal 
        built-in function line_regression to find out the line, and 
        judge the position of the car by the middle position of the 
        line. It will take this to decide whether to go straight or 
        turn left/right. Once it verifies its decision, the OpenMV 
        would send RPC instructions to the mbed, and then the mbed 
        program can operate the corresponsing codes according to it.
    
    P3_AprilTag:
        This program is similar to the line regression. The bbcar 
        receives the RPC instructions from OpenMV. OpenMV would decide
        whether to go ahead, rotate car, or turn left/right.

Note:
    My Ping seems not to work, so I didn't use that.
