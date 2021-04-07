#include "mbed.h"


DigitalOut led1(LED1);
InterruptIn sw2(USER_BUTTON);
EventQueue queue(32 * EVENTS_EVENT_SIZE);

Thread t;

void led1_info() {
   // Note that printf is deferred with a call in the queue
   // It is not executed in the interrupt context
   printf("led1 is triggered! \r\n");
}

void Trig_led1()  {
   // Execute the time critical part first
   led1 = !led1;
   // The rest can execute later in user context
   queue.call(led1_info);
}

int main() {
   // t is a thread to process tasks in an EventQueue
   // call in queue is now executed in the context of the thread
   // t call queue.dispatch_forever() to start the scheduler of the EventQueue
   t.start(callback(&queue, &EventQueue::dispatch_forever));

   // 'Trig_led1' will execute in IRQ context
   sw2.rise(Trig_led1);
}
