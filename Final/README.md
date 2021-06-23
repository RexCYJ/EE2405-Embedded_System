Class:      EE2405, Embedded System Lab.
Date:       June. 23, 2021.
Filename:   Final Project, Integrated BBCar Navigating System
Author:     108061121,  Yu-Jia, Chen.

Content:    README.md
            main.cpp
            Control.py
            XbeeSetting.py
            bbcar control package
            RPC function package

Connect:    OpenMV   tx: D1,     rx: D0
            Xbee     tx: D10,    rx: D9
            pcUSB    tx: USBTX,  rx: USBRX
            Right servo: D6
            Left  servo: D5

Code: 
    1) main.cpp:
        This is the main function that operates on the mbed and drives the bbcar. It uses a RPC call loop to wait for the messages from XBee, which is connected on PC, and another RPC call functions to contact with OpenMV. 
        At first, the bbcar waits for the start signal (RPC call) from PC to start the navigating system. As soon as the car receives the start signal, it jumps to the other RPC loop in the  thread "OpenMVRPC", and wait for the instructions from OpenMV.

        a) OpenMV RPC call loop:
            void TrackRPCLoop();
            RPCFunction rpcTrack(&Track, "start");

            void Taskfinish(Arguments *in, Reply *out);
            RPCFunction rpcTaskfinish(&Taskfinish, "finish");

            void Track(Arguments *in, Reply *out);
        
        b) Car motion control function:
            void carDrift(Arguments *in, Reply *out);
            void carCircle(Arguments *in, Reply *out);
            void carStop(Arguments *in, Reply *out);
            void carTurn(Arguments *in, Reply *out);
            void carStraight(Arguments *in, Reply *out);
            RPCFunction rpcCarDrift(&carDrift, "drift");
            RPCFunction rpcCarCircle(&carCircle, "circle");
            RPCFunction rpcCarStop(&carStop, "stop");
            RPCFunction rpcCarTurn(&carTurn, "turn");
            RPCFunction rpcCarStraight(&carStraight, "straight");

    2) OpenMV main.py:
        This python code is operated on OpenMV, and mainly deals with the motion of bbcar. Adjusted from Lab14, the code contains two parts: apriltag and line detection. 
        In the outer while loop, it first searches for the apriltag in the its vision, and distinguishes the tag's id, and then asks the bbcar to operate the corresponsding code. After that, I do some process on the image to find out the guide line in the screen. Then, with the assistance of the built-in function, line_regression(), we can find out the line's direction. The car uses the horizontal center (x_center) of the line as the reference to judge whether to go straight, turn left, or turn right.
    
    3) Control.py:
        This is the commander operated on the PC, and is used to communicate with bbcar through Xbee, including sending the start signal, and receiving the tasks reports from the bbcar.

    4) bbcar and bbcar_RPC codes package:
        These are the codes offered in Lab13, but some parts of it were modified to make the car much more easy and powerful to control.
    
    5) XbeeSetting.py:
        Just for setting the IDs of xbees.

Reference:
    https://www.ee.nthu.edu.tw/ee240500/
    https://openmv.io/blogs/news/linear-regression-line-following

Conclusion:
    I shouldn't have selected this course.