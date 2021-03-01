#include "mbed.h"

DigitalOut myLED1(LED1);
DigitalOut myLED3(LED3);

void Led(DigitalOut &ledpin, int n);

int main()
{
   myLED1 = 1;
   myLED3 = 1;
   
   while (true)
   {
       myLED1.write(0);
       Led(myLED3, 6);

       myLED3.write(0);
       Led(myLED1, 4);
   }
}

void Led(DigitalOut &ledpin, int n)
{
   for (int i = 0; i < n; ++i)
   {                     //blink for 10 times
      ledpin = !ledpin; // toggle led
      ThisThread::sleep_for(300ms);
   }
}