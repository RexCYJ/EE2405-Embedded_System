#include "mbed.h"

#define CENTER_BASE 1495   // right wheel

// left wheel   1495
// right wheel  1535

PwmOut servo(D11);

void servo_control(int speed) {
   if (speed > 200)       speed = 200;
   else if (speed < -200) speed = -200;

   servo = (CENTER_BASE + speed)/20000.0f;
}

int main() {
   servo.period_ms(20);

   while(1) {
      servo_control(10);
      ThisThread::sleep_for(2000ms);
      servo_control(0);
      ThisThread::sleep_for(2000ms);
   }
}
