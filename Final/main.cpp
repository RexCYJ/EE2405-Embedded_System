#include "mbed.h"
#include "bbcar_rpc.h"
#include "bbcar.h"
#include <chrono>
#include <math.h>

// UART Connection
BufferedSerial OpenMV(D1,D0);       // tx,rx
BufferedSerial Xbee(D10,D9);        // tx,rx
BufferedSerial pc(USBTX, USBRX);        // tx,rx

// Boe Bot Car Setting
Thread OpenMVRPC(osPriorityHigh);
EventQueue OpenMvEvent(32 * EVENTS_EVENT_SIZE);
Ticker servo_ticker;                            // Call parallax_servo::control() every 5ms
PwmOut pin5(D5), pin6(D6);                      // D5: Left servo, D6: Right servo
BBCar car(pin5, pin6, servo_ticker);

DigitalOut led3(LED3);
DigitalOut led2(LED2);

// OpenMV Command Server
void Track(Arguments *in, Reply *out);
void TrackRPCLoop();
RPCFunction rpcTrack(&Track, "start");

void Taskfinish(Arguments *in, Reply *out);
RPCFunction rpcTaskfinish(&Taskfinish, "finish");

// Car Control function
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

volatile int Track_State;
volatile int EventID;

FILE *devin  = fdopen(&Xbee, "r");
FILE *devout = fdopen(&Xbee, "w");
FILE *OMin  = fdopen(&OpenMV, "r");
FILE *OMout = fdopen(&OpenMV, "w");

int main(void)
{
    OpenMV.set_baud(9600);
    Xbee.set_baud(9600);
    pc.set_baud(9600);

    char buf[256], outbuf[256];
    
    car.stop();
    Track_State = 0;
    OpenMVRPC.start(callback(&OpenMvEvent, &EventQueue::dispatch_forever));
    
    while (1) {
        memset(buf, 0, 256);
        for( int i = 0; ; i++ ) {
            char recv = fgetc(devin);
            if(recv == '\n') {
                break;
            }
            buf[i] = recv;
        }
        RPC::call(buf, outbuf);
    }
    return 0;
}

void Track(Arguments *in, Reply *out)
{
    Track_State = 1;
    EventID = OpenMvEvent.call(&TrackRPCLoop);
}

// XBEE RPC call "Start" here
void TrackRPCLoop()
{
    ThisThread::sleep_for(2s);
    char buf[256], outbuf[256];
    while (1) {
        OpenMV.write("1", 1);
        if (Track_State == 0) break;
        memset(buf, 0, 256);
        for( int i = 0; ; i++ ) {
            char recv = fgetc(OMin);
            if(recv == '\n') {
                break;
            }
            buf[i] = recv;
        }
        RPC::call(buf, outbuf);   
    }
}

void carCircle(Arguments *in, Reply *out)
{
    led3 = 1;
    double speed = 100;
    // int turnTime = 500;
    // int straightTime = 2000;


    Xbee.write("Rounding\r\n", 20);

    // First    <--------------
    car.turn(-speed, 1, 0);
    ThisThread::sleep_for(1400ms);
    car.stop();
    ThisThread::sleep_for(200ms);
    car.goStraight(-speed);
    ThisThread::sleep_for(1300ms);
    car.stop();
    ThisThread::sleep_for(100ms);

    // Second   |^|
    car.turn(-speed, -0.7, 0.7);
    ThisThread::sleep_for(1000ms);
    car.stop();
    ThisThread::sleep_for(200ms);
    car.goStraight(-speed);
    ThisThread::sleep_for(3500ms);
    car.stop();
    ThisThread::sleep_for(200ms);

    // Third    ----------->
    car.turn(-speed, -0.7, 0.7);
    ThisThread::sleep_for(1100ms);
    car.stop();
    ThisThread::sleep_for(200ms);
    car.goStraight(-speed);
    ThisThread::sleep_for(3300ms);
    car.stop();
    ThisThread::sleep_for(100ms);

    // Forth   |v|
    car.turn(-speed, -0.5, 0.7);
    ThisThread::sleep_for(1000ms);
    car.stop();
    ThisThread::sleep_for(200ms);
    car.goStraight(-speed);
    ThisThread::sleep_for(600ms);

    car.stop();
    ThisThread::sleep_for(200ms);
    // Xbee.write("Rounding Finish\r\n", 34);
    
    led3 = 0;
}

void carStraight(Arguments *in, Reply *out)
{
    led3 = 1;

    double speed;
    speed = in->getArg<double>();
    car.goStraight(-speed);
    ThisThread::sleep_for(50ms);
    
    led3 = 0;
}

void carTurn(Arguments *in, Reply *out)
{
    led3 = 1;
    double speed, rfactor, lfactor;
    
    speed = in->getArg<double>();
    rfactor = in->getArg<double>();
    lfactor = in->getArg<double>();

    car.turn(-speed, rfactor, lfactor);
    ThisThread::sleep_for(50ms);
    
    led3 = 0;
}

void carDrift(Arguments *in, Reply *out)
{
    led3 = 1;

    double speed;

    speed = in->getArg<double>();;

    Xbee.write("Drifting\r\n", 20);
    car.turn(-50, -0.5, 1);
    ThisThread::sleep_for(1000ms);
    car.stop();
    ThisThread::sleep_for(200ms);
    car.goStraight(-speed);
    ThisThread::sleep_for(1s);
    car.turn(-speed, 1, 0.5);
    ThisThread::sleep_for(4s);
   
    car.stop();
    ThisThread::sleep_for(200ms);
    // Taskfinish();

    led3 = 0;
}

void carStop(Arguments *in, Reply *out)
{
    led2 = 1;
    car.stop();
    ThisThread::sleep_for(50ms);
    
    led2 = 0;
}

void Taskfinish(Arguments *in, Reply *out)
{
    OpenMvEvent.cancel(EventID);
    Track_State = 0;
    Xbee.write("Finish\r\n", 16);
}