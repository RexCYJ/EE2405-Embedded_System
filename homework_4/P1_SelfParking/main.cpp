#include "mbed.h"
#include "bbcar_rpc.h"
#include "bbcar.h"
#include <chrono>

#include <math.h>

using namespace std::chrono;

// BufferedSerial OpenMV(D1,D0); //tx,rx
BufferedSerial Xbee(D10,D9); //tx,rx

Thread Carthread(osPriorityHigh);
EventQueue parking;

DigitalOut my_led1(LED3);

Ticker servo_ticker;
PwmOut pin5(D5), pin6(D6);
BBCar car(pin5, pin6, servo_ticker);

void SelfParking(double, double, double);
void SetPosition(Arguments *in, Reply *out);
RPCFunction rpcSetpose(&SetPosition, "setpose");

int main(void)
{
    // OpenMV.set_baud(9600);
    Xbee.set_baud(9600);
    
    char buf[256], outbuf[256];
    FILE *devin  = fdopen(&Xbee, "r");
    FILE *devout = fdopen(&Xbee, "w");
    car.stop();

    Carthread.start(callback(&parking, &EventQueue::dispatch_forever));
    
    // Xbee Initialization
    // char xbee_reply[4];
    // Xbee.write("+++", 3);
    // Xbee.read(&xbee_reply[0], sizeof(xbee_reply[0]));
    // Xbee.read(&xbee_reply[1], sizeof(xbee_reply[1]));
    // if(xbee_reply[0] == 'O' && xbee_reply[1] == 'K'){
    //    printf("enter AT mode.\r\n");
    //    xbee_reply[0] = '\0';
    //    xbee_reply[1] = '\0';
    // }
    // Xbee.write("ATMY 123\r\n", 12);
    // Xbee.write("ATDL 456\r\n", 12);
    // Xbee.write("ATID 0x1\r\n", 10);
    // Xbee.write("ATDL\r\n", 6);
    // Xbee.write("ATCN\r\n", 6);
    // my_led1 = 1;
    // Xbee Initialization

    
    while (1) {
        my_led1 = 1;
        memset(buf, 0, 256);
        for( int i = 0; ; i++ ) {
            char recv = fgetc(devin);
            if(recv == '\n') {
                printf("\r\n");
                break;
            }
            buf[i] = fputc(recv, devout);
        }
        RPC::call(buf, outbuf);
    }
}

void SetPosition(Arguments *in, Reply *out)
{
    // out->putData("Mbed$ Start parking\n");
    double d1 = in->getArg<double>();
    double d2 = in->getArg<double>();
    double dir = in->getArg<double>();
    parking.call(&SelfParking, d1, d2, dir);
}

void SelfParking(double d1, double d2, double dir)
{
    my_led1 = 0;
    double rotate_coef = 12;
    double straight_coef = 100;
    double straight_coef2 = 120;
    double speed = 0;

    ThisThread::sleep_for(2s);

    // correct the angle
    if (dir) {
        car.turn(60, -1, -dir);
        ThisThread::sleep_for(chrono::milliseconds(int(dir * rotate_coef)));
        car.stop();
    }
    
    ThisThread::sleep_for(1s);

    // decide to go forward or backward
    if (d2 > 0) speed = -50;
    else if (d2 < 0) speed = 50;
    car.goStraight(speed);              // go
    ThisThread::sleep_for(chrono::milliseconds(int((d2 + 11) * straight_coef)));
    car.stop();

    ThisThread::sleep_for(1s);

    // rotate 90 degree
    if (dir == 0) dir = 1;
    car.turn(60, -1, dir);
    ThisThread::sleep_for(chrono::milliseconds(int(95 * rotate_coef)));
    car.stop();

    ThisThread::sleep_for(1s);

    // go into the garage
    car.goStraight(-50);
    ThisThread::sleep_for(chrono::milliseconds(int((d1+15) * straight_coef2)));
    car.stop();
    printf("End\n");
}

