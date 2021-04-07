#include "mbed.h"

#define MAXIMUM_BUFFER_SIZE 1

static DigitalOut led1(LED1); //led1 = PA_5
static DigitalOut led2(LED2); //led2 = PB_14

Thread thread1;
Thread thread2;

static BufferedSerial pc(USBTX, USBRX); // tx, rx
static BufferedSerial device1(D10, D9); // tx, rx  D10:tx  D9:rx
static BufferedSerial device2(D1, D0);  // tx, rx   D1:tx   D0:rx

void master_thread(){
   while (1){
      char buf[MAXIMUM_BUFFER_SIZE] = {0};
      if (pc.readable()){
         pc.read(buf, 1);
         device1.write(buf, 1);
      }
      else{
         char buf1[MAXIMUM_BUFFER_SIZE] = {'0'};
         device1.write(buf1, 1);
      }

   ThisThread::sleep_for(5ms);
   }
}

void slave_thread(){
   led1 = 0;
   led2 = 0;

   while (1){
      char buf2[MAXIMUM_BUFFER_SIZE] = {0};
      if (device2.readable()){
         device2.read(buf2, 1);

         if (buf2[0] == '1'){
            led1 = 1;
            led2 = 0;
         }
         else if (buf2[0] == '2'){
            led1 = 0;
            led2 = 1;
         }
      else{
         led1 = 0;
         led2 = 0;
      }
      pc.write(buf2, 1);
      }
   }
}

int main(){
   // Set desired properties (9600-8-N-1).
   device1.set_baud(9600);
   device1.set_format(
      /* bits */ 8,
      /* parity */ BufferedSerial::None,
      /* stop bit */ 1);

   // Set desired properties (9600-8-N-1).
   device2.set_baud(9600);
   device2.set_format(
      /* bits */ 8,
      /* parity */ BufferedSerial::None,
      /* stop bit */ 1);

   thread1.start(master_thread);
   thread2.start(slave_thread);
}