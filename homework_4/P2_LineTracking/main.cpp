#include "mbed.h"
#include "bbcar_rpc.h"
#include "bbcar.h"
#include <chrono>
#include <math.h>

#define SCREEN_DISTORTION_RATIO 0.714
#define SCREEN_Y_CORRECTION     7
#define SCREEN_REFERENCE_Y      8
#define RUNTIME                 50ms
// #define ROTATE_RUNTIME          200ms

// using namespace std::chrono;

// UART Connection
BufferedSerial OpenMV(D1,D0);       // tx,rx
BufferedSerial Xbee(D10,D9);        // tx,rx
BufferedSerial pc(USBTX, USBRX);        // tx,rx

// Boe Bot Car Setting
// Thread RunThread(osPriorityHigh);               // For lineDetection
Thread OpenMVRPC(osPriorityHigh);
// EventQueue MoveEvent(32 * EVENTS_EVENT_SIZE);   // EventQueue fro RunThread
EventQueue OpenMvEvent(32 * EVENTS_EVENT_SIZE);
Ticker servo_ticker;                            // Call parallax_servo::control() every 5ms
PwmOut pin5(D5), pin6(D6);                      // D5: Left servo, D6: Right servo
BBCar car(pin5, pin6, servo_ticker);

// Debug
DigitalOut my_led1(LED3);
DigitalOut my_led2(LED2);

// RPC call function
void LineDetect();
void Drive();
void OperateLoop();
void Track_stop(Arguments *in, Reply *out);
void Track(Arguments *in, Reply *out);

void TrackRPCLoop();

void carStop(Arguments *in, Reply *out);
void carTurn(Arguments *in, Reply *out);
void carStraight(Arguments *in, Reply *out);
RPCFunction rpcCarStop(&carStop, "stop");
RPCFunction rpcCarStraight(&carStraight, "straight");
RPCFunction rpcCarTurn(&carTurn, "turn");

RPCFunction rpcTrack(&Track, "track");
// RPCFunction rpcTrackStop(&Track_stop, "stop");

// Global Variable
volatile int Track_State;
volatile int EventID_line, EventID_move, EventID_Loop;
volatile double X1, Y1, X2, Y2;

FILE *devin  = fdopen(&Xbee, "r");
FILE *devout = fdopen(&Xbee, "w");
FILE *OMin  = fdopen(&OpenMV, "r");
FILE *OMout = fdopen(&OpenMV, "w");

//////////////////////// Main Function ////////////////////////////////////////////////////
int main(void)
{
    OpenMV.set_baud(9600);
    Xbee.set_baud(9600);
    // pc.set_baud(9600);

    char buf[256], outbuf[256];
    
    car.stop();

    Track_State = 0;
    // RunThread.start(callback(&MoveEvent, &EventQueue::dispatch_forever));
    OpenMVRPC.start(callback(&OpenMvEvent, &EventQueue::dispatch_forever));
    
    while (1) {
        my_led1 = 1;
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

////////////////////////// Called by Xbee ////////////////////////////////
void Track(Arguments *in, Reply *out)
{
    Track_State = 1;
    OpenMvEvent.call(&TrackRPCLoop);
}

void TrackRPCLoop()
{
    // if (Track_State == 0) break;
    ThisThread::sleep_for(2s);
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
    my_led1 = 1;
    car.stop();
    ThisThread::sleep_for(50ms);
    my_led1 = 0;
}

void carTurn(Arguments *in, Reply *out)
{
    my_led1 = 1;
    double speed, rfactor, lfactor;
    
    speed = in->getArg<double>();
    rfactor = in->getArg<double>();
    lfactor = in->getArg<double>();

    car.turn(-speed, rfactor, lfactor);       // turn right
    ThisThread::sleep_for(50ms);
    my_led1 = 0;
}

void carStraight(Arguments *in, Reply *out)
{
    my_led1 = 1;
    double speed;
    speed = in->getArg<double>();
    car.goStraight(-speed);       // turn right
    ThisThread::sleep_for(50ms);
    my_led1 = 0;
}

/*
void OperateLoop()
{
    printf("StartLoop\r\n");
    ThisThread::sleep_for(2s);
    double x1, x2, y1, y2;
    while (Track_State) {
        // ThisThread::sleep_for(30ms);
        LineDetect();
        x1 = X1; x2 = X2; y1 = Y1; y2 = Y2;
        ThisThread::sleep_for(30ms);
        LineDetect();
        x1 += X1; x2 += X2; y1 += Y1; y2 += Y2;
        // ThisThread::sleep_for(20ms);
        // LineDetect();
        // x1 += X1; x2 += X2; y1 += Y1; y2 += Y2;
        X1 = x1/2; X2 = x2/2; Y1 = y1/2; Y2 = y2/2;
        Drive();
    }
}

void LineDetect()
{
    my_led1 = 0;
    char ch[1], cstring[20];
    int buf[4], i = 0, j = 0, k, temp = 0;
    // buf[0] = buf[1] = buf[2] = buf[3] = 0;
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
            if (cstring[j] == ' ' || cstring[j] == '\0') {buf[i++] = temp; temp = 0;}
            else {
                temp *= 10;
                temp += cstring[j] - 48;
            }
            j++;
        }
        printf("BUF:  %d %d %d %d\r\n", buf[0], buf[1], buf[2], buf[3]);
        if (buf[0] >= 0 && buf[0] <= 160 && buf[1] >= 0 && buf[1] <= 160 && buf[2] >= 0 && buf[2] <= 120 && buf[3] >= 0 && buf[3] <= 120)
            break;
        // else car.Setspeed(30);
    }
    
    my_led1 = 1;
    
    X1 = buf[0]; X2 = buf[1];
    Y1 = buf[2]; Y2 = buf[3];

//     X1 = (X1 - 80) / ((Y1 - SCREEN_REFERENCE_Y) * SCREEN_DISTORTION_RATIO * 2);
//     Y1 = (120 - Y1) / 70 * SCREEN_Y_CORRECTION;
//     X2 = (X2 - 80) / ((Y2 - SCREEN_REFERENCE_Y) * SCREEN_DISTORTION_RATIO * 2);
//     Y2 = (120 - Y2) / 70 * SCREEN_Y_CORRECTION;
//     printf("Pos:  %lf %lf %lf %lf\r\n", X1, X2, Y1, Y2);
}

void Drive()
{
    my_led1 = 0;

    if (Track_State == 0)
        return;

    double side_coef = 4;
    double side_rotate = 0.8;
    double speed = 50;
    double x_center, y_center;
    double x_vector, y_vector;
    double x1, x2, y1, y2;
    double theta;
    double STRAIGHT_ANGLE_RANGE = 20;
    double STRAIGHT_CENTER_REGION = 55;
    double ROTATE_RUNTIME = 2;
    double ANGLE_COEF = 0.2;
    double y0crossing;

    // if (Y1 > Y2) X1 

    x1 = X1 - 80;       // ^ y
    x2 = X2 - 80;       // |-----> x 
    y1 = 120 - Y1;
    y2 = 120 - Y2;
    x_center = (x1 + x2)/2;
    y_center = (y1 + y2)/2;
    if (y1 > y2) {
        y_vector = y1 - y2;
        x_vector = x1 - x2;
    } else {
        y_vector = y2 - y1;
        x_vector = x2 - x1;    
    }
                                                          //           |theta
    theta = (atan(x_vector / y_vector) * 180 / 3.1416);   // in degree |/

    y0crossing = x1 * (-y_vector/x_vector) + y1;

    if ((y0crossing > 60 && y0crossing < 300) || (fabs(x1) < 40 && fabs(x2) < 40)) {
        if (fabs(x_center) < 30) {
            car.goStraight(-speed);
            ThisThread::sleep_for(200ms);
        } else if (x_center > 0) {           // align right side
            car.turn(-speed, side_rotate, 1);       // turn right
            ThisThread::sleep_for(chrono::milliseconds(int(fabs(x_center) * side_coef * 2)));
            // car.turn(-speed, 1, side_rotate);        // turn left
            // ThisThread::sleep_for(chrono::milliseconds(int(fabs(x_center) * side_coef)));
        } else if (x_center < 0) {           // align left side
            car.turn(-speed, 1, side_rotate);        // turn left
            ThisThread::sleep_for(chrono::milliseconds(int(fabs(x_center) * side_coef * 2)));
            // car.turn(-speed, side_rotate, 1);       // turn right
            // ThisThread::sleep_for(chrono::milliseconds(int(fabs(x_center) * side_coef)));   
        }
    // } else if (th) {

    } else if (theta > -15 && x_center > 0) {             // turn right
        car.turn(-speed, 0.3f, 1);        
        ThisThread::sleep_for(200ms);   
        car.stop();
    } else if (theta < 15 && x_center < 0) {
        car.turn(-speed, 1, 0.3f);         // turn left
        ThisThread::sleep_for(200ms);   
        car.stop();
    } else {
        car.Setspeed(-30);
        ThisThread::sleep_for(20ms);
        // car.stop();
    }


    // if (Track_State) {
    //     if ((x_center < 50 && x_center > 0 && theta < 0) || (x_center > -50 && x_center < 0 && theta > 0)) {
    //         printf(">> Straight line, theta:  %lf\r\n", theta);
    //         if (fabs(x_center) < STRAIGHT_CENTER_REGION) {
    //             car.goStraight(-speed);
    //             ThisThread::sleep_for(RUNTIME);
    //             // car.stop();
    //         } else if (x_center > STRAIGHT_CENTER_REGION) {    // S-shape move, align left side
    //             car.turn(-speed, 0.7, 1);       // turn left
    //             ThisThread::sleep_for(chrono::milliseconds(int(fabs(x_center) * side_coef)));
    //             // car.turn(-speed, 0.3, -1);      // turn right
    //             // ThisThread::sleep_for(chrono::milliseconds(int(fabs(x_center) * side_coef)));
    //             car.stop();
    //         } else if (x_center < -STRAIGHT_CENTER_REGION) {
    //             car.turn(-speed, 0.7, -1);      // turn right
    //             ThisThread::sleep_for(chrono::milliseconds(int(fabs(x_center) * side_coef)));
    //             // car.turn(-speed, 0.5, 1);       // turn left
    //             // ThisThread::sleep_for(chrono::milliseconds(int(fabs(X1) * side_coef)));
    //             car.stop();
    //         } else
    //             printf("BUG!\r\n");
    //     } else if (theta > 30) {   
    //         printf(">> Turn right theta: %lf\r\n", theta);
    //         // car.goStraight(-speed);
    //         // ThisThread::sleep_for(chrono::milliseconds(int(abs(y_center) * ROTATE_RUNTIME)));
    //         car.turn(-speed, 0.7 - fabs(theta)/90, 1);
    //         ThisThread::sleep_for(chrono::milliseconds(int(fabs(theta) * ANGLE_COEF)));
    //         car.stop();
    //     } else if (theta < -40) {
    //         printf(">> Turn left theta: %lf\r\n", theta);
    //         // car.goStraight(-speed);
    //         // ThisThread::sleep_for(chrono::milliseconds(int(fabs(y_center) * ROTATE_RUNTIME)));
    //         car.turn(-speed, 0.7 - fabs(theta)/90, -1);
    //         ThisThread::sleep_for(chrono::milliseconds(int(fabs(theta) * ANGLE_COEF)));
    //         car.stop();
    //     } else
    //         car.stop();
    
    // }
    my_led1 = 1;
}
*/