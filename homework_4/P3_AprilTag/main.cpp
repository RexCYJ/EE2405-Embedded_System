#include "mbed.h"
#include "bbcar_rpc.h"
#include "bbcar.h"
#include <chrono>
#include <math.h>

using namespace std::chrono;

BufferedSerial OpenMV(D1,D0);           // tx,rx
BufferedSerial Xbee(D10,D9);            // tx,rx
BufferedSerial pc(USBTX, USBRX);        // tx,rx

// Boe Bot Car Setting
Thread RunThread(osPriorityHigh);               // For lineDetection
EventQueue MoveEvent(32 * EVENTS_EVENT_SIZE);   // EventQueue fro RunThread
Ticker servo_ticker;                            // Call parallax_servo::control() every 5ms
PwmOut pin5(D5), pin6(D6);                      // D5: Left servo, D6: Right servo
BBCar car(pin5, pin6, servo_ticker);

// Debug
DigitalOut led2(LED2);
DigitalOut led3(LED3);

// Ping
DigitalInOut ping(D11);

// RPC call function
void ReadOpenMV();
void adjust();

void OpenMVRPC();
void AprilTag(Arguments *in, Reply *out);
void carAdjust(Arguments *in, Reply *out);
void carStop(Arguments *in, Reply *out);
void carTurn(Arguments *in, Reply *out);
void carStraight(Arguments *in, Reply *out);
RPCFunction rpcCarAdjust(&carAdjust, "adjust");
RPCFunction rpcCarStop(&carStop, "stop");
RPCFunction rpcCarStraight(&carStraight, "straight");
RPCFunction rpcCarTurn(&carTurn, "turn");

RPCFunction rpcApriltag(&AprilTag, "apriltag");

Timer t;

volatile int Track_State = 0;
volatile int X, Z, Yangle;

FILE *devin  = fdopen(&Xbee, "r");
FILE *devout = fdopen(&Xbee, "w");
FILE *OMin  = fdopen(&OpenMV, "r");
FILE *OMout = fdopen(&OpenMV, "w");

int main(void)
{
    OpenMV.set_baud(9600);
    Xbee.set_baud(9600);
    // pc.set_baud(9600);

    char buf[256], outbuf[256];
    
    car.stop();

    led2 = 0;
    led3 = 0;
    RunThread.start(callback(&MoveEvent, &EventQueue::dispatch_forever));
    
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
}

void AprilTag(Arguments *in, Reply *out)
{
    Track_State = 1;
    printf("Start\r\n");
    MoveEvent.call(&OpenMVRPC);
}

void OpenMVRPC()
{
    char buf[256], outbuf[256];
    while (1) {
        memset(buf, 0, 256);
        for( int i = 0; ; i++ ) {
            char recv = fgetc(OMin);
            if(recv == '\n') {
                break;
            }
            buf[i] = recv;
        }
        RPC::call(buf, outbuf);
        // if (Track_State == 0) break;
    }
}

void carStop(Arguments *in, Reply *out)
{
    led2 = 1;
    car.stop();
    ThisThread::sleep_for(50ms);
    led2 = 0;
}

void carTurn(Arguments *in, Reply *out)
{
    led2 = 1;
    double speed, factor;
    int Rtime;
    
    speed = in->getArg<double>();
    factor = in->getArg<double>();
    Rtime = in->getArg<int>();

    car.turn(-speed, -factor, factor);  // factor 1: right
    ThisThread::sleep_for(60ms);
    car.stop();
    led2 = 0;
}

void carStraight(Arguments *in, Reply *out)
{
    led2 = 1;
    double speed;
    speed = in->getArg<double>();
    car.goStraight(-speed);
    ThisThread::sleep_for(100ms);
    car.stop();
    led2 = 0;
}

void carAdjust(Arguments *in, Reply *out)
{
    led2 = 1;
    double speed, direction;
    int TurnTime, StraightSpeed;

    speed = in->getArg<double>();
    direction = in->getArg<double>();
    TurnTime = in->getArg<int>();
    StraightSpeed = in->getArg<int>();

    car.turn(-speed, -direction, direction);    // dir 1: right; dir 0: left
    ThisThread::sleep_for(900ms);
    car.stop();
    ThisThread::sleep_for(200ms);
    car.goStraight(-StraightSpeed);                     // straight
    ThisThread::sleep_for(1500ms);
    car.turn(-speed, direction, -direction);    // dir 1: right; dir 0: left
    ThisThread::sleep_for(900ms);
    car.stop();

    led2 = 0;
}

void adjust()
{
    double Xavg, Zavg, Yangavg;
    double speed = 50;
    double rotate_coef = 1;
    double forward_coef = 1;
    double distance;
    float val;

    int Xs = 0, Zs = 0, Yangles = 0;
    
    for (int i = 0; i < 5; i++) {
        ReadOpenMV();
        Xs += X; Zs += Z; Yangles += Yangle;
    }

    Xavg = Xs/5.0;
    Zavg = Zs/5.0;
    Yangavg = Yangles/5.0;
    printf("X: %f, Z: %f, Ry: %f\r\n", Xavg, Zavg, Yangavg);

    /*///////////////////////
    ping.output();
    ping = 0; wait_us(200);
    ping = 1; wait_us(5);
    ping = 0; wait_us(5);

    ping.input();
    while(ping.read() == 0);
    t.start();
    while(ping.read() == 1);
    val = t.read();
    distance = val*17700.4f;
    t.stop();
    t.reset();
    *////////////////////////

    // printf("distance: %lf\r\n", distance);

    led3 = 1;
    if (Xavg > 1) {
        // rotate
        car.turn(-speed, 0, 1);
        ThisThread::sleep_for(chrono::milliseconds(int(fabs(Xavg)*rotate_coef)));
        car.stop();
    } else if (Xavg < -1) {
        car.turn(-speed, 1, 0);
        ThisThread::sleep_for(chrono::milliseconds(int(fabs(Xavg)*rotate_coef)));
        car.stop();
    }
    ThisThread::sleep_for(100ms);
    car.goStraight(-speed);
    ThisThread::sleep_for(chrono::milliseconds(int(Zavg * forward_coef)));
    car.stop();

    ThisThread::sleep_for(100ms);
    if (Yangavg > 15) {
        car.turn(-speed, -1, 1);
        ThisThread::sleep_for(chrono::milliseconds(int(fabs(90 - Yangavg)*rotate_coef)));
        car.goStraight(-speed);
        ThisThread::sleep_for(chrono::milliseconds(int(fabs(Yangavg) * forward_coef)));
        car.turn(-speed, 1, -1);
        ThisThread::sleep_for(chrono::milliseconds(int(fabs(90)*rotate_coef)));
    } else if (Yangavg < -15) {
        car.turn(-speed, 1, -1);
        ThisThread::sleep_for(chrono::milliseconds(int(fabs(90 + Yangavg)*rotate_coef)));
        car.goStraight(-speed);
        ThisThread::sleep_for(chrono::milliseconds(int(fabs(Yangavg) * forward_coef)));    
        car.turn(-speed, -1, 1);
        ThisThread::sleep_for(chrono::milliseconds(int(fabs(Yangavg)*rotate_coef)));
    }
    car.stop();
    led3 = 0;
}

void ReadOpenMV()
{
    led2 = 1;
    char ch[1], cstring[20];
    int buf[3], i = 0, j = 0, k, temp = 0;
    int factor = 1;
    while (1) {
        while (1) {             // read in the string
            if (OpenMV.readable()) {
                j = 0;
                do {
                    OpenMV.read(ch, sizeof(ch));
                    cstring[j++] = ch[0];
                } while (ch[0] != '\n');
                k = j;
                cstring[k - 1] = '\0';
                break;
            }
        }
        // printf("%s\r\n", cstring);
        i = j = 0;
        while (j < k) {     // analysis the string
            if (cstring[j] == ' ' || cstring[j] == '\0') {
                buf[i++] = factor * temp; 
                factor = 1; temp = 0;
            }
            else if (cstring[j] == '-') {factor = -1;}
            else {
                temp *= 10;
                temp += cstring[j] - 48;
            }
            j++;
        }
        printf("BUF:  %d %d %d\r\n", buf[0], buf[1], buf[2]);
        if (buf[0] >= -10 && buf[0] <= 10 && buf[1] <= 0 && buf[1] >= -15 && 
            buf[2] >= 0 && buf[2] <= 360)
            break;
    }
    X = buf[0]; 
    Z = buf[1];
    if (buf[2] > 180) Yangle = buf[2] - 360;
    else Yangle = buf[2];

    led2 = 0;
}

