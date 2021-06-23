import sensor, image, time, pyb, math

enable_lens_corr = False # turn on for straighter lines...

sensor.reset()
sensor.set_pixformat(sensor.GRAYSCALE) # grayscale is faster
sensor.set_framesize(sensor.QQVGA)
sensor.set_vflip(True)
sensor.set_hmirror(True)
sensor.skip_frames(time = 2000)
sensor.set_auto_gain(False)  	 # must turn this off to prevent image washout...
sensor.set_auto_whitebal(False)  # must turn this off to prevent image washout...
clock = time.clock()

# All lines also have `x1()`, `y1()`, `x2()`, and `y2()` methods to get their end-points
# and a `line()` method to get all the above as one 4 value tuple for `draw_line()`.
# Note! Unlike find_qrcodes the find_apriltags method does not need lens correction on the image to work.

uart = pyb.UART(3,9600,timeout_char=1000)
uart.init(9600,bits=8,parity = None, stop=1, timeout_char=1000)

f_x = (2.8 / 3.984) * 160 # find_apriltags defaults to this if not set
f_y = (2.8 / 2.952) * 120 # find_apriltags defaults to this if not set
c_x = 160 * 0.5 # find_apriltags defaults to this if not set (the image.w * 0.5)
c_y = 120 * 0.5 # find_apriltags defaults to this if not set (the image.h * 0.5)

StraightSpeed = 150.0
TurnSpeed = 50.0
StraightRange = 30
LineMagnitudeTH = 10

def degrees(radians):
   return (180 * radians) / math.pi

uart.write(("/straight/run %f\n" %StraightSpeed).encode())
trigger = 1
BlockNum = 0

while(True):
    trigger = uart.read()
    #trigger = 1
    clock.tick()
    img = sensor.snapshot().histeq()
    tag = 0
    if trigger != None:
        for tag in img.find_apriltags(fx=f_x, fy=f_y, cx=c_x, cy=c_y): # defaults to TAG36H11
            img.draw_rectangle(tag.rect(), color = (255, 0, 0))
            img.draw_cross(tag.cx(), tag.cy(), color = (0, 255, 0))
            RyDeg = degrees(tag.y_rotation())
            if (RyDeg > 180):
                RyDeg -= 360
            print_args1 = (int(tag.x_translation()), int(tag.z_translation()), int(RyDeg), tag.id())
            print(print_args1)
            if (abs(tag.x_translation()) > 1.3 and tag.id() == 5):
                print('For turn')
                turn_coef = (30, -tag.x_translation()/10, 0)
                uart.write(("/turn/run %f %f %f\n" %turn_coef).encode())
                break
            elif (tag.z_translation() > -8 and tag.id() == 5):
                uart.write(("/circle/run\n").encode())
            elif (tag.id() == 83):
                BlockNum +=1
                print('Find block')
                if (BlockNum >= 2):
                    uart.write(("/stop/run\n").encode())
                    uart.write(("/finish/run \n").encode())
                    break
                uart.write(("/drift/run %f\n" %200.0).encode())
                #uart.write(("/stop/run\n").encode())
                #turn_coef = (70, -1, 0.7)
                #uart.write(("/turn/run %f %f %f\n" %turn_coef).encode())
                #uart.write(("/straight/run %f\n" %StraightSpeed).encode())

        img.binary([(245,255)])
        img.erode(1)
        l = 0;
        l = img.get_regression([(255, 255)], robust=True)
        if (l != 0):
            #dspl = math.cos(math.radians(l.theta() + 180)) * -l.rho()
            img.draw_line(l.line(), color=127)
            x_center = (l.x1() + l.x2())/2
            x_center -= 80
            print_args = (l.x1(), l.x2(), l.y1(), l.y2(), l.magnitude(), x_center)
            print('x1: %f x2: %f y1: %f y2: %f Mag: %f x_center: %f' %print_args)
            if (l.magnitude() > LineMagnitudeTH and x_center < StraightRange and x_center > -StraightRange):
                print('Line straight')
                uart.write(("/straight/run %f\n" %StraightSpeed).encode())
            elif (l.magnitude() > LineMagnitudeTH and x_center >= StraightRange):       # turn right
                rfactor = 0.8 - x_center/100
                print('Line turn')
                turn_coef = (TurnSpeed, rfactor, 1)
                uart.write(("/turn/run %f %f %f\n" %turn_coef).encode())
            elif (l.magnitude() > LineMagnitudeTH and x_center <= -StraightRange):       # turn left
                print('Line turn')
                lfactor = 0.8 + x_center/100
                turn_coef = (TurnSpeed, 1, lfactor)
                uart.write(("/turn/run %f %f %f\n" %turn_coef).encode())
        else:
            uart.write(("/stop/run\n").encode())

    time.sleep_ms(30)

