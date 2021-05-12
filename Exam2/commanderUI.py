import paho.mqtt.client as paho
import serial
import time

# serial port
serdev = '/dev/ttyACM0'
s = serial.Serial(serdev, 9600)

# MQTT broker hosted on local machine
mqttc = paho.Client() 
cmdline = 'idle'
# cmddict = {'idle': 0, 'gesture': 1, 'detectAngle': 2}
IsClientLooping = 0
times = 0
Maxtimes = 20
tiltAngle = [0]*Maxtimes

################################################################
# Settings for connection
# TODO: revise host to your IP
host = "192.168.160.45"
topic = "THRESHOLD"
cmdtopic = "PYTHON"
TFclass = [0]*Maxtimes
Selfclass = [0]*Maxtimes

# Callbacks
def on_connect(self, mosq, obj, rc):
    print("Py>> Connected rc: " + str(rc))

# Reaction as python receives message
def on_message(mosq, obj, msg):
    global IsClientLooping
    global times
    global Maxtimes
    global tiltAngle
    global cmdline
    # line = s.readline()
    # print("Mbed$ ", line)
    line = s.readline()
    print("Mbed$ ", line)
    print("Py>> " + msg.payload + '\n')
    times += 1
    if (times > Maxtimes):
        s.write(bytes('/' + cmdline + '/run\n', 'UTF-8'))
        IsClientLooping = 0
        for i in range(Maxtimes):
            num = s.readline()
            TFclass[i] = int(num)
        for i in range(Maxtimes):
            num = s.readline()
            Selfclass[i] = int(num)
        
    # elif (cmdline == 'detectAngle'):
    #     print("Py>> Received overtilted angle: #", str(times), ' ', str(msg.payload), '\n')
    #     tiltAngle[times] = msg.payload
    #     times += 1
    #     if (times >= Maxtimes):
    #         ret = mqttc.publish(msg.topic, "Stop sending\n", qos=0)
    #         if (ret[0] != 0):
    #             print("Py>> Publish failed")
    #         for i in range(Maxtimes):
    #             print("Py>>  #" + str(i) + " angle: " + str(tiltAngle[i]) + '\n')
    #         times = 0
    #         IsClientLooping = 0

def on_subscribe(mosq, obj, mid, granted_qos):
    print("Py>> Subscribed OK")

def on_unsubscribe(mosq, obj, mid, granted_qos):
    print("Py>> Unsubscribed OK")

# Set callbacks
mqttc.on_message = on_message
mqttc.on_connect = on_connect
mqttc.on_subscribe = on_subscribe
mqttc.on_unsubscribe = on_unsubscribe

# Connect and subscribe
print("Py>> Connecting to " + host + "/" + topic)
mqttc.connect(host, port=1883, keepalive=60)
mqttc.subscribe(topic, 0)
###########################################################################

line = s.readline()
print("Mbed$ ", line)
line = s.readline()
print("Mbed$ ", line)
line = s.readline()
print("Mbed$ ", line)
line = s.readline()
print("Mbed$ ", line)
line = s.readline()
print("Mbed$ ", line)

while True:
    cmdline = input('Py>> ')    # mode 1: gesture; mode 2: detectAngle
    s.write(bytes('/' + cmdline + '/run\n', 'UTF-8'))
    IsClientLooping = 1
    
    line = s.readline()
    print("Mbed$ ", line)
    line = s.readline()
    print("Mbed$ ", line)

    while IsClientLooping:
        mqttc.loop()
    
    # line = s.readline()
    # print("Mbed$ ", line)

# # Publish messages from Python
# num = 0
# while num != 5:
#     ret = mqttc.publish(topic, "Message from Python!\n", qos=0)
#     if (ret[0] != 0):
#             print("Publish failed")
#     mqttc.loop()
#     time.sleep(1.5)
#     num += 1

# Loop forever, receiving messages
mqttc.loop_forever()