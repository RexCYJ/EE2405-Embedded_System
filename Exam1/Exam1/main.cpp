#include "mbed.h"
#include "uLCD_4DGL.h"

#define SAMPLES_PER_PERIOD  1000

using namespace std::chrono;

Timer debounce; 

AnalogOut Vout(PA_4);                   // DAC, output for generated wave
AnalogIn Vin(D6);                       // ADC, receive the wave after pass through the filter
InterruptIn BTNup(D3);                  // Button for select freq, moving cursor upward
InterruptIn BTNdown(D4);                // Button for select freq, moving cursor downward
InterruptIn BTNcomf(D5);                // Button for confirm the signal and start receive Vin
DigitalOut DBG1(LED3);                  // Debug LED
DigitalOut DBG2(LED2);                  // Debug LED

uLCD_4DGL uLCD(D1, D0, D2);             // uLCD ports

Thread Output(osPriorityHigh);              // (High prior) generate output wave in this thread
Thread Input(osPriorityHigh2);              // (Higher prior) receive input wave, and change cursor in this thread

int usrrate[4] = {8, 4, 2, 1};      // the freq we want to perform
int usrStep[4] = {80, 40, 20, 10};

int preuLCDcursor = 0;                      // last position of cursor
volatile int uLCDcursor = 0;                // position of cursor
volatile int curStep;                       // current freq
volatile float vouticrm;
volatile float InputWave[1000];             // Store the received wave samples
volatile int IWindex = 0;
const float Vmax = 3.0f/3.3f;
volatile float curVout;

void Main_init();                   // initialize 
void Button_up();
void Button_down();
void Button_confirm();
void MenuCursor();                  // renew the cursor on uLCD
void SamplingIndicator();           // indicate the sampling process on uLCD
void Wavegen();                     // generate wave
void Waveread();                    // read wave
void WaveTrans();                   // transmiss sampled data to PC

int main()
{
    Main_init();
    Output.start(callback(&queue, &EventQueue::dispatch_forever));
    Input.start(callback(&sample, &EventQueue::dispatch_forever));
    queue.call_every(1ms, Wavegen);
    while (1) {}
}

void Main_init()
{
    DBG1 = 1; DBG2 = 1;
    debounce.start();
    BTNup.rise(&Button_up);
    BTNdown.rise(&Button_down);
    BTNcomf.rise(&Button_confirm);
    
    uLCD.background_color(0x000000);
    uLCD.textbackground_color(0x000000);
    uLCD.cls();
    uLCD.color(0x00FF00);
    uLCD.text_height(2);    uLCD.text_width(2);
    uLCD.locate(0, 0);
    uLCD.printf("Rate:");
    uLCD.color(0xF0F000);
    uLCD.text_width(2); uLCD.text_height(2);
    uLCD.locate(2, 2);
    uLCD.printf("1/8 Hz");
    uLCD.locate(2, 3);
    uLCD.printf("1/4 Hz");
    uLCD.locate(2, 4);
    uLCD.printf("1/2 Hz");
    uLCD.locate(2, 5);
    uLCD.printf("1   Hz");
    uLCD.color(0xFF0000);
    uLCD.locate(0, 2+uLCDcursor);
    uLCD.printf(">");

    curStep = usrStep[0];
    vouticrm = (3.0f / 3.3f) / ((SAMPLES_PER_PERIOD / usrStep) - 1);
}

void MenuCursor()
{
    DBG1 = !DBG1;
    uLCD.locate(0, 2+preuLCDcursor);
    uLCD.printf(" ");
    uLCD.locate(0, 2+uLCDcursor);
    uLCD.printf(">");
    preuLCDcursor = uLCDcursor;
}

void SamplingIndicator()
{
    uLCD.locate(0, 2+uLCDcursor);
    uLCD.printf("@");
}

void Button_up()
{
    if (duration_cast<milliseconds>(debounce.elapsed_time()).count() > DEBOUNCE_PERIOD) {
        debounce.reset();
        DBG2 = !DBG2;
        if (uLCDcursor > 0) uLCDcursor -= 1;
        curStep = usrStep[uLCDcursor];                 // change current freq
        vouticrm = (Vmax) / ((SAMPLES_PER_PERIOD / curStep) - 1);
        sample.call(MenuCursor);
    }
}

void Button_down()
{
    if (duration_cast<milliseconds>(debounce.elapsed_time()).count() > DEBOUNCE_PERIOD) {
        debounce.reset();
        DBG1 = !DBG1;
        if (uLCDcursor < 3) uLCDcursor += 1;
        curStep = usrStep[uLCDcursor];                 // change current freq
        vouticrm = Vmax / ((SAMPLES_PER_PERIOD / curStep) - 1);
        sample.call(MenuCursor);
    }
}

void Button_confirm()
{
    if (duration_cast<milliseconds>(debounce.elapsed_time()).count() > DEBOUNCE_PERIOD) {
        debounce.reset();
        DBG1 = !DBG1;
        
        sample.call(SamplingIndicator);         // change the indicator
        sample.call(Waveread);                  // start sampling for a second 
        sample.call_in(1s, WaveTrans);          // read the wave after finish reading
        sample.call_in(1s, MenuCursor);         // reset the cursor
    }
}

void Wavegen()
{
    for (curVout = 0; curVout < Vmax; curVout += vouticrm) {
        Vout = curVout;
        ThisThread::sleep_for(1ms);
    }
    for (int i = 0; i < 240 - 2 * curStep; i++) {
        Vout = Vmax;
        ThisThread::sleep_for(1ms);
    }
    for (curVout = Vmax; curVout > vouticrm; curVout -= vouticrm) {
        Vout = curVout;
        ThisThread::sleep_for(1ms);
    }
}

void Waveread()
{
    for (int i = 0; i < 1000; i++) {
        InputWave[IWindex++] = float(Vin);
        ThisThread::sleep_for(1ms);
    }
}

void WaveTrans()
{
    for (int j = 0; j < IWindex; j++)
        printf("%f\r\n", InputWave[j]);
    IWindex = 0;
}

