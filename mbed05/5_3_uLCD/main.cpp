#include "mbed.h"
#include "uLCD_4DGL.h"

uLCD_4DGL uLCD(D1, D0, D2);

int main()
{
      //uLCD.printf("\nHello uLCD World\n"); //Default Green on black text
      //ThisThread::sleep_for(30s);
      uLCD.background_color(0xFFFFFF);
      uLCD.locate(5, 10);
      uLCD.color(0x0000FF);
      uLCD.printf("108061121");
      uLCD.locate(100, 100);
      uLCD.color(0x00FF00);
      uLCD.set_font(FONT_12X16);
      for (int i = 0; i < 30; i++) {
            uLCD.printf("%d", i);
            ThisThread::sleep_for(1s);
      }
}