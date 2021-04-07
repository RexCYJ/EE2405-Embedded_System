#include "mbed.h"

DigitalOut led2(LED2);
InterruptIn sw3(USER_BUTTON);
EventQueue queue(32 * EVENTS_EVENT_SIZE);

Thread t;

void Trig_led2()  {
   led2 = !led2;
   // Safe to use 'printf' in context of thread 't', while IRQ is not.
   printf("led2 is triggered! \r\n");
}


int main() {

   t.start(callback(&queue, &EventQueue::dispatch_forever));

   // 'Trig_led2' is put directly into the queue (same as queue.call)
   // 'Trig_led2' will execute in context of thread 't'
   // The interrupt event simply processes the queue.call(), instead of 'Trig_led2'
   sw3.rise(queue.event(Trig_led2));
}