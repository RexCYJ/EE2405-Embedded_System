import sensor, image, time, pyb
enable_lens_corr = False # turn on for straighter lines...
sensor.reset()
sensor.set_pixformat(sensor.GRAYSCALE) # grayscale is faster
sensor.set_vflip(True)
sensor.set_hmirror(True)
sensor.set_framesize(sensor.QQVGA)
sensor.skip_frames(time = 2000)
clock = time.clock()

# All lines also have `x1()`, `y1()`, `x2()`, and `y2()` methods to get their end-points
# and a `line()` method to get all the above as one 4 value tuple for `draw_line()`.

uart = pyb.UART(3,9600,timeout_char=1000)
uart.init(9600,bits=8,parity = None, stop=1, timeout_char=1000)

StraightSpeed = 50.0
TurnSpeed = 50.0
StraightRange = 40

while(True):
    clock.tick()
    img = sensor.snapshot().histeq()
    img.binary([(240,255)])
    img.erode(1)
    if enable_lens_corr: img.lens_corr(1.8) # for 2.8mm lens...

    l = img.get_regression([(255, 255)], robust=True)
    if l:
        print(l)
        img.draw_line(l.line(), color=127)
        print_args = (l.x1(), l.x2(), l.y1(), l.y2())
        x_center = (l.x1() + l.x2())/2
        x_center -= 80
        if (x_center < StraightRange and x_center > -StraightRange):
            uart.write(("/straight/run %f\n" %StraightSpeed).encode())
        elif (x_center >= StraightRange):       # turn right
            rfactor = 0.8 - x_center/100
            turn_coef = (TurnSpeed, rfactor, 1)
            uart.write(("/turn/run %f %f %f\n" %turn_coef).encode())
        elif (x_center <= -StraightRange):       # turn left
            lfactor = 0.8 + x_center/100
            turn_coef = (TurnSpeed, 1, lfactor)
            uart.write(("/turn/run %f %f %f\n" %turn_coef).encode())

   ##print("FPS %f" % clock.fps())
