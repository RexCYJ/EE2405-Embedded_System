Class:      EE2405, Embedded System Lab.
Date:       May. 18, 2021. 
Filename:   Homework 3.
Author:     108061121,  Yu-Jia, Chen.

Content:    Readme.txt
            main.cpp
            commanderUI.py
            FFTanalysis.py
            wifi_mqtt source codes
            tensorflow source codes
            mbed RPC source codes
            BSP_B-L475E-IOT01 source codes
            uLCD source codes

Connect:    uLCD(RX: D0, TX: D1, RES: D2);

Description:
    首先mbed程式會在RPC迴圈中等待Python傳來的使用者指令，再根據
    收到的RPC指令進入對應的函數。若是收到gesture，會到對應的RPC
    function將gestureSettin丟入thread執行，在這個函數，會使用
    tensorflow的程式去偵測使用者的手勢，根據手勢去選擇Threshold 
    angle，確認選擇後，按下User button，將Threshold angle傳
    回Python端，然後python會傳一個RPC指令去結束目前Thread內
    EventQueue執行的函數，然後返回一開始的RPCloop。
    
    若使用者輸入detectAngle在Python端的介面，mbed會執行偵測角度
    的模式。一開始會用500毫秒偵測目前的參考重力值，接著開始根據參考
    值去運算目前mbed的傾角，並在uLCD輸出偵測到的值，若超過我們設定
    的Threshold angle，就會透過mqtt回傳給python，當pyhton收到
    超過既定數量的回傳，會再傳RPC指令給mbed使其結束目前thread內的
    程式，回到原本的RPC loop。

Results:
    Please take a look of the demo video.