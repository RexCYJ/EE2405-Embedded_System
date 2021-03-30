// EE2405, homework 2
// Mar. 30, 2021.
// 108061121, Y.J.Chen

#include "mbed.h"
#include "uLCD_4DGL.h"

#define SAMPLES_PER_PERIOD  1000        // Number of Samples per second
#define NUM_FREQ            5           // Number of chosen output frequency
#define DEBOUNCE_PERIOD     600         // debounce time, in ms 

using namespace std::chrono;

Timer debounce;                         // debounce time counter
AnalogOut Vout(PA_4);                   // DAC, output for generated wave
AnalogIn Vin(D6);                       // ADC, receive the wave after pass through the filter
DigitalOut DBG1(LED3);                  // Debug LED
DigitalOut DBG2(LED2);                  // Debug LED

InterruptIn BTNup(D3);                  // Button for select freq, moving cursor upward
InterruptIn BTNdown(D4);                // Button for select freq, moving cursor downward
InterruptIn BTNcomf(D5);                // Button for confirm the signal and start receive Vin
uLCD_4DGL uLCD(D1, D0, D2);             // uLCD ports
Thread Output(osPriorityHigh);              // (High prior) generate output wave in this thread
Thread Input(osPriorityHigh2);              // (Higher prior) receive input wave, and change cursor in this thread
EventQueue queue(32 * EVENTS_EVENT_SIZE);   // arrange tasks to generate wave
EventQueue sample(32 * EVENTS_EVENT_SIZE);  // tasks handling cursor-moving, wave-receiving

int userfreq[NUM_FREQ] = {100, 80, 50, 20, 5};      // the freq we want to perform

int preuLCDcursor = 0;                      // last position of cursor
volatile int uLCDcursor = 0;                // position of cursor
volatile int curFreq;                       // current freq
volatile float vouticrm;
volatile float InputWave[1000];             // Store the received wave samples
volatile int IWindex = 0;
const float Vmax = 3.0f/3.3f;               // magnitude of the output analog signal
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
    
    while(1) {

    }
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
    uLCD.printf("Freq:");
    uLCD.color(0xF0F000);
    uLCD.text_width(2); uLCD.text_height(2);
    uLCD.locate(2, 2);
    uLCD.printf("%d Hz", userfreq[0]);
    uLCD.locate(2, 3);
    uLCD.printf("%d  Hz", userfreq[1]);
    uLCD.locate(2, 4);
    uLCD.printf("%d  Hz", userfreq[2]);
    uLCD.locate(2, 5);
    uLCD.printf("%d  Hz", userfreq[3]);
    uLCD.locate(2, 6);
    uLCD.printf("%d   Hz", userfreq[4]);
    uLCD.color(0xFF0000);
    uLCD.locate(0, 2+uLCDcursor);
    uLCD.printf(">");

    curFreq = userfreq[0];
    vouticrm = (3.0f / 3.3f) / ((SAMPLES_PER_PERIOD / curFreq) - 1);
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
        if (uLCDcursor == 0) uLCDcursor = NUM_FREQ - 1;
        else    uLCDcursor -= 1;
        curFreq = userfreq[uLCDcursor];                 // change current freq
        vouticrm = (3.0f / 3.3f) / ((SAMPLES_PER_PERIOD / curFreq) - 1);
        sample.call(MenuCursor);
    }
}

void Button_down()
{
    if (duration_cast<milliseconds>(debounce.elapsed_time()).count() > DEBOUNCE_PERIOD) {
        debounce.reset();
        DBG1 = !DBG1;
        if (uLCDcursor == NUM_FREQ-1) uLCDcursor = 0;
        else    uLCDcursor += 1;
        curFreq = userfreq[uLCDcursor];                 // change current freq
        vouticrm = (3.0f / 3.3f) / ((SAMPLES_PER_PERIOD / curFreq) - 1);
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