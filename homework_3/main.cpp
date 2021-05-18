#include "mbed.h"
#include <math.h>

#include "mbed_rpc.h"
#include "uLCD_4DGL.h"

#include "stm32l475e_iot01_accelero.h"

#include "accelerometer_handler.h"
#include "tfconfig.h"
#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/micro/kernels/micro_ops.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"

#include "magic_wand_model_data.h"
#include "MQTTNetwork.h"
#include "MQTTmbed.h"
#include "MQTTClient.h"

// -------------------- Constant Definition ------------------------
#define THRESHOLD_ANGLE_NUM 4

// ---------------------- GLOBAL VARIABLES -------------------------
// 1) Wifi and MQTT
WiFiInterface *wifi = WiFiInterface::get_default_instance();
NetworkInterface* net = wifi;
MQTTNetwork mqttNetwork(net);
MQTT::Client<MQTTNetwork, Countdown> client(mqttNetwork);
volatile int message_num = 0;
volatile int arrivedcount = 0;
volatile bool closed = false;
const char* topic_threshold = "THRESHOLD";      // MQTT TOPIC
Thread mqtt_thread(osPriorityHigh);
EventQueue mqtt_queue;

// 2) RPC
BufferedSerial pc(USBTX, USBRX);
void GestureUI(Arguments *in, Reply *out);
void AngleDetect(Arguments *in, Reply *out);
void BackToMode0(Arguments *in, Reply *out);
RPCFunction rpcGesture(&GestureUI, "gesture");
RPCFunction rpcDetect(&AngleDetect, "detectAngle");
RPCFunction rpcBacktoMode0(&BackToMode0, "back");

// 3) LED
DigitalOut my_led1(LED1);
DigitalOut my_led2(LED2);
DigitalOut my_led3(LED3);

// 4) BUTTON
InterruptIn btnRecord(USER_BUTTON);
void ButtonInterrupt();

// 5) TENSORFLOW
constexpr int kTensorArenaSize = 60 * 1024;
uint8_t tensor_arena[kTensorArenaSize];
int PredictGesture(float* output);
volatile int gesture_index = label_num;
Thread gesture_thread(osPriorityNormal);
Thread detect_thread(osPriorityNormal);
EventQueue gesture_queue;
EventQueue detect_queue;
volatile int GestureEventCode;
volatile int DetectEventCode;
int GestureSetting();

// 6) ACCELERO
int16_t posDataXYZ[3] = {0};

// 7) uLCD
uLCD_4DGL uLCD(D1, D0, D2);             // uLCD ports
void uLCD_modeSelect();
void uLCD_gestureInit();
void uLCD_gestureNext();
void uLCD_tiltAngleInit();
void uLCD_tiltAngleShow(double);
Thread uLCD_thread(osPriorityHigh);
EventQueue uLCD_queue;

// 8) SYSTEM STATUS
volatile int system_mode = 0;
volatile int selected_gesture = 0;
const double thresholdAngle[THRESHOLD_ANGLE_NUM] = {30, 45, 60, 75};
volatile double curThresAngle = 0;
void tiltDetect();
const double Pi = 3.14159265;

void messageArrived(MQTT::MessageData& md) {       // react when receive TOPIC
   system_mode = 0;        // end gesture setting mode
   my_led2 = 0;   
   my_led1 = 0;
   uLCD_queue.call(&uLCD_modeSelect);
   ThisThread::sleep_for(500ms);
}

void publish_message(MQTT::Client<MQTTNetwork, Countdown>* client, double val) {
   message_num++;
   MQTT::Message message;
   char buff[100];
   if (system_mode == 1) {
      sprintf(buff, "%f", thresholdAngle[selected_gesture]);
   } else if (system_mode == 2) {
      sprintf(buff, "%f", val);
   }
   message.qos = MQTT::QOS0;
   message.retained = false;
   message.dup = false;
   message.payload = (void*) buff;
   message.payloadlen = strlen(buff) + 1;
   int rc = client->publish(topic_threshold, message);

   printf("Puslish message: %f\r\n", val);
}

void close_mqtt() {
    closed = true;
}

int main() {
   // ========================================================
   printf("Start Main\r\n");
   if (!wifi) {
      printf("ERROR: No WiFiInterface found.\r\n");
      return -1;
   }

   printf("\nConnecting to %s...\r\n", MBED_CONF_APP_WIFI_SSID);                 // #1 line
   int ret = wifi->connect(MBED_CONF_APP_WIFI_SSID, MBED_CONF_APP_WIFI_PASSWORD, 
                           NSAPI_SECURITY_WPA_WPA2);
   if (ret != 0) {
      printf("\nConnection error: %d\r\n", ret);
      return -1;
   }

   //TODO: revise host to your ip
   const char* host = "192.168.0.103";    // setting WIFI address

   printf("Connecting to TCP network...\r\n");                                // #2 line
   SocketAddress sockAddr;
   sockAddr.set_ip_address(host);
   sockAddr.set_port(1883);

   printf("address is %s/%d\r\n",                                             // #3 line
            (sockAddr.get_ip_address() ? sockAddr.get_ip_address() : "None"), 
            (sockAddr.get_port() ? sockAddr.get_port() : 0) );                //check setting

   int rc = mqttNetwork.connect(sockAddr);//(host, 1883);
   if (rc != 0) {
      printf("Connection error.");
      return -1;
   }
   printf("Successfully connected!\r\n");                                     // #4 line

   MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
   
   data.MQTTVersion = 3;
   data.clientID.cstring = "Mbed";

   if ((rc = client.connect(data)) != 0){
      printf("Fail to connect MQTT\r\n");
   }
   if (client.subscribe(topic_threshold, MQTT::QOS0, messageArrived) != 0){
      printf("Fail to subscribe\r\n");
   }
   // ==========================================================

   BSP_ACCELERO_Init();       // initial accelerometer

   // ==========================================================
   // RPC INITIAL
   char buf[256], outbuf[256];
   FILE *devin = fdopen(&pc, "r");
   FILE *devout = fdopen(&pc, "w");
   
   // ==========================================================
   // THREAD, ISR, EVENTQUEUE
   mqtt_thread.start(callback(&mqtt_queue, &EventQueue::dispatch_forever));
   uLCD_thread.start(callback(&uLCD_queue, &EventQueue::dispatch_forever));
   gesture_thread.start(callback(&gesture_queue, &EventQueue::dispatch_forever));
   detect_thread.start(callback(&detect_queue, &EventQueue::dispatch_forever));
   btnRecord.rise(&ButtonInterrupt);
   
   // ==========================================================
   // uLCD_initial();
   uLCD_modeSelect();

   my_led3 = 0;
   my_led2 = 0;   
   my_led1 = 0;
   // ==========================================================

   // MAIN LOOP FUNCTION
   while (true) {
      
      // RPC Loop, waiting for python user function
      memset(buf, 0, 256);                   // clear buffer
      for(int i=0; i<255; i++) {
         char recv = fgetc(devin);
         if (recv == '\r' || recv == '\n') {
            // printf("\r\n");
            break;
         }
         buf[i] = recv;
      }
      RPC::call(buf, outbuf);
      // printf("Receive: %s\r\n", outbuf);
   }

   // ======================================================
   // WIFI TERMINATE
   printf("Ready to close MQTT Network......\n");
   if ((rc = client.unsubscribe(topic_threshold)) != 0)
      printf("Failed: rc from unsubscribe was %d\n", rc);
   if ((rc = client.disconnect()) != 0)
      printf("Failed: rc from disconnect was %d\n", rc);
   mqttNetwork.disconnect();
   printf("Successfully closed!\n");

   return 0;
}

// Gesture predict function
int PredictGesture(float* output) {
  // How many times the most recent gesture has been matched in a row
  static int continuous_count = 0;
  // The result of the last prediction
  static int last_predict = -1;

  // Find whichever output has a probability > 0.8 (they sum to 1)
  int this_predict = -1;
  for (int i = 0; i < label_num; i++) {
    if (output[i] > 0.8) this_predict = i;
  }

  // No gesture was detected above the threshold
  if (this_predict == -1) {
    continuous_count = 0;
    last_predict = label_num;
    return label_num;
  }

  if (last_predict == this_predict) {
    continuous_count += 1;
  } else {
    continuous_count = 0;
  }
  last_predict = this_predict;

  // If we haven't yet had enough consecutive matches for this gesture,
  // report a negative result
  if (continuous_count < config.consecutiveInferenceThresholds[this_predict]) {
    return label_num;
  }
  // Otherwise, we've seen a positive result, so clear all our variables
  // and report it
  continuous_count = 0;
  last_predict = -1;

  return this_predict;
}

// display the screen of mode 1
void uLCD_modeSelect()
{
   uLCD.background_color(0x000000);
   uLCD.textbackground_color(0x000000);
   uLCD.cls();
   uLCD.color(0xFFFF00);
   uLCD.text_height(1);    uLCD.text_width(1);
   uLCD.locate(0, 2);
   uLCD.printf("RPC instruction:");
   uLCD.color(0xFFFFFF);
   uLCD.locate(0, 5);
   uLCD.printf("1 Select threshold");
   uLCD.locate(0, 8);
   uLCD.printf("2 Detect the angle");
   uLCD.locate(3, 11);
   uLCD.printf("current Th:\n  %lf", curThresAngle);
}

void uLCD_gestureInit()
{
   uLCD.background_color(0x000000);
   uLCD.textbackground_color(0x000000);
   uLCD.cls();
   uLCD.color(0x0000FF);
   uLCD.text_height(1);    uLCD.text_width(1);
   uLCD.locate(0, 0);
   uLCD.printf("Threshold:");
   uLCD.color(0xFFFFFF);
   uLCD.locate(11, 2);   uLCD.printf(">  30");
   uLCD.locate(14, 3);   uLCD.printf("45");
   uLCD.locate(14, 4);   uLCD.printf("60");
   uLCD.locate(14, 5);   uLCD.printf("75");
}

void uLCD_gestureNext()
{
   uLCD.locate(11,(selected_gesture - 1 + THRESHOLD_ANGLE_NUM)%THRESHOLD_ANGLE_NUM + 2);
   uLCD.printf(" ");
   uLCD.locate(11, selected_gesture + 2);
   uLCD.printf(">");
}

// Button interrupt function
void ButtonInterrupt()
{
   if (system_mode == 1) {
      system_mode = 0;
      mqtt_queue.call(&publish_message, &client, 1.0);
      curThresAngle = thresholdAngle[selected_gesture];       // store the selected threshold angle
   }
}

// RPC call function
void GestureUI(Arguments *in, Reply *out)
{  
   uLCD_queue.call(&uLCD_gestureInit);
   GestureEventCode = gesture_queue.call(&GestureSetting);
}

// RPC call function
void AngleDetect(Arguments *in, Reply *out)
{
   uLCD_queue.call(&uLCD_tiltAngleInit);
   DetectEventCode = detect_queue.call(&tiltDetect);
}

void BackToMode0(Arguments *in, Reply *out)
{
   system_mode = 0;        // end gesture setting mode
   my_led2 = 0;   
   my_led1 = 0;
   
   uLCD_queue.call(&uLCD_modeSelect);
   gesture_queue.cancel(GestureEventCode);
   detect_queue.cancel(DetectEventCode);
}

// Gesture Detect Loop
// Called by RPC Gesture UI. Thrown into gesture thread
int GestureSetting()
{
   my_led1 = 1;
   system_mode = 1;
   // Whether we should clear the buffer next time we fetch data
   bool should_clear_buffer = false;
   bool got_data = false;

   // Set up logging.
   static tflite::MicroErrorReporter micro_error_reporter;
   tflite::ErrorReporter* error_reporter = &micro_error_reporter;

   // Map the model into a usable data structure. This doesn't involve any
   // copying or parsing, it's a very lightweight operation.
   const tflite::Model* model = tflite::GetModel(g_magic_wand_model_data);
   if (model->version() != TFLITE_SCHEMA_VERSION) {
      error_reporter->Report(
         "Model provided is schema version %d not equal "
         "to supported version %d.",
         model->version(), TFLITE_SCHEMA_VERSION);
      return -1;
   }

   // Pull in only the operation implementations we need.
   // This relies on a complete list of all the ops needed by this graph.
   // An easier approach is to just use the AllOpsResolver, but this will
   // incur some penalty in code space for op implementations that are not
   // needed by this graph.
   static tflite::MicroOpResolver<6> micro_op_resolver;
   micro_op_resolver.AddBuiltin(
      tflite::BuiltinOperator_DEPTHWISE_CONV_2D,
      tflite::ops::micro::Register_DEPTHWISE_CONV_2D());
   micro_op_resolver.AddBuiltin(tflite::BuiltinOperator_MAX_POOL_2D,
                               tflite::ops::micro::Register_MAX_POOL_2D());
   micro_op_resolver.AddBuiltin(tflite::BuiltinOperator_CONV_2D,
                               tflite::ops::micro::Register_CONV_2D());
   micro_op_resolver.AddBuiltin(tflite::BuiltinOperator_FULLY_CONNECTED,
                               tflite::ops::micro::Register_FULLY_CONNECTED());
   micro_op_resolver.AddBuiltin(tflite::BuiltinOperator_SOFTMAX,
                               tflite::ops::micro::Register_SOFTMAX());
   micro_op_resolver.AddBuiltin(tflite::BuiltinOperator_RESHAPE,
                               tflite::ops::micro::Register_RESHAPE(), 1);

   // Build an interpreter to run the model with
   static tflite::MicroInterpreter static_interpreter(
      model, micro_op_resolver, tensor_arena, kTensorArenaSize, error_reporter);
   tflite::MicroInterpreter* interpreter = &static_interpreter;

   // Allocate memory from the tensor_arena for the model's tensors
   interpreter->AllocateTensors();

   // Obtain pointer to the model's input tensor
   TfLiteTensor* model_input = interpreter->input(0);
   if ((model_input->dims->size != 4) || (model_input->dims->data[0] != 1) ||
         (model_input->dims->data[1] != config.seq_length) ||
         (model_input->dims->data[2] != kChannelNumber) ||
         (model_input->type != kTfLiteFloat32)) {
      error_reporter->Report("Bad input tensor parameters in model");
      return -1;
   }

   int input_length = model_input->bytes / sizeof(float);

   TfLiteStatus setup_status = SetupAccelerometer(error_reporter);
   if (setup_status != kTfLiteOk) {
    error_reporter->Report("Set up failed\n");
    return -1;
   }

   error_reporter->Report("Set up successful...\n");
   
   while (system_mode == 1) {
      // Attempt to read new data from the accelerometer
      got_data = ReadAccelerometer(error_reporter, model_input->data.f,
                                 input_length, should_clear_buffer);

      // If there was no new data,
      // don't try to clear the buffer again and wait until next time
      if (!got_data) {
         should_clear_buffer = false;
         continue;
      }

      // Run inference, and report any error
      TfLiteStatus invoke_status = interpreter->Invoke();
      if (invoke_status != kTfLiteOk) {
         error_reporter->Report("Invoke failed on index: %d\n", begin_index);
         continue;
      }

      // Analyze the results to obtain a prediction
      gesture_index = PredictGesture(interpreter->output(0)->data.f);

      // Clear the buffer next time we read data
      should_clear_buffer = gesture_index < label_num;

      // Produce an output
      if (gesture_index < label_num) {
         selected_gesture = (selected_gesture + 1) % THRESHOLD_ANGLE_NUM;
         uLCD_queue.call(&uLCD_gestureNext);
         ThisThread::sleep_for(300ms);
      }
   }
   return 0;
}

void tiltDetect()
{
   // detect reference gravity
   my_led2 = 1;
   system_mode = 2;
   my_led3 = 1;    // reference acceleration detection
   int16_t refpData[3];
   ThisThread::sleep_for(500ms);
   BSP_ACCELERO_AccGetXYZ(refpData);
   ThisThread::sleep_for(500ms);
   my_led3 = 0;    // finish detect;
   double angle = 0;
   double ThresholdAngle = curThresAngle;

   // printf("Ref Angle %d %d %d\n", refpData[0], refpData[1], refpData[2]);

   while (system_mode == 2) {
      BSP_ACCELERO_AccGetXYZ(posDataXYZ);
      ThisThread::sleep_for(400ms);
      angle = acos(((refpData[2]>posDataXYZ[2])?posDataXYZ[2]:refpData[2])*1.0/refpData[2]) * 180 / Pi;
      if (angle > ThresholdAngle) {
         mqtt_queue.call(&publish_message, &client, angle);
      }
      uLCD_queue.call(&uLCD_tiltAngleShow, angle);
      ThisThread::sleep_for(200ms);
   }
}

void uLCD_tiltAngleInit()
{
   uLCD.background_color(0x000000);
   uLCD.textbackground_color(0x000000);
   uLCD.cls();
   uLCD.color(0x0000FF);
   uLCD.text_height(1);    uLCD.text_width(1);
   uLCD.locate(0, 0);
   uLCD.printf("Angle detecting...");
   uLCD.color(0xFFFFFF);
   uLCD.locate(2, 2);
   uLCD.printf("Current Angle:");
}

void uLCD_tiltAngleShow(double angle)
{
   uLCD.locate(3, 3);
   uLCD.printf("            ");
   uLCD.locate(3, 3);
   uLCD.printf("%lf", angle);
}