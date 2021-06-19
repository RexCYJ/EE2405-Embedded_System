import sensor, image, time, math, pyb

sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QQVGA) # we run out of memory if the resolution is much bigger...
sensor.set_vflip(True)
sensor.set_hmirror(True)
sensor.skip_frames(time = 2000)
sensor.set_auto_gain(False)  # must turn this off to prevent image washout...
sensor.set_auto_whitebal(False)  # must turn this off to prevent image washout...
clock = time.clock()

# Note! Unlike find_qrcodes the find_apriltags method does not need lens correction on the image to work.

# What's the difference between tag families? Well, for example, the TAG16H5 family is effectively
# a 4x4 square tag. So, this means it can be seen at a longer distance than a TAG36H11 tag which
# is a 6x6 square tag. However, the lower H value (H5 versus H11) means that the false positve
# rate for the 4x4 tag is much, much, much, higher than the 6x6 tag. So, unless you have a
# reason to use the other tags families just use TAG36H11 which is the default family.

# The AprilTags library outputs the pose information for tags. This is the x/y/z translation and
# x/y/z rotation. The x/y/z rotation is in radians and can be converted to degrees. As for
# translation the units are dimensionless and you must apply a conversion function.

# f_x is the x focal length of the camera. It should be equal to the lens focal length in mm
# divided by the x sensor size in mm times the number of pixels in the image.
# The below values are for the OV7725 camera with a 2.8 mm lens.

# f_y is the y focal length of the camera. It should be equal to the lens focal length in mm
# divided by the y sensor size in mm times the number of pixels in the image.
# The below values are for the OV7725 camera with a 2.8 mm lens.

# c_x is the image x center position in pixels.
# c_y is the image y center position in pixels.

f_x = (2.8 / 3.984) * 160 # find_apriltags defaults to this if not set
f_y = (2.8 / 2.952) * 120 # find_apriltags defaults to this if not set
c_x = 160 * 0.5 # find_apriltags defaults to this if not set (the image.w * 0.5)
c_y = 120 * 0.5 # find_apriltags defaults to this if not set (the image.h * 0.5)

uart = pyb.UART(3,9600,timeout_char=1000)
uart.init(9600,bits=8,parity = None, stop=1, timeout_char=1000)

StraightSpeed = 50.0
TurnSpeed = 50.0
StraightRange = 40
AngleTH = 15
RotateCoef = 20
StraightCoef = 20

def degrees(radians):
   return (180 * radians) / math.pi

while(True):
    clock.tick()
    img = sensor.snapshot()
    for tag in img.find_apriltags(fx=f_x, fy=f_y, cx=c_x, cy=c_y): # defaults to TAG36H11
        img.draw_rectangle(tag.rect(), color = (255, 0, 0))
        img.draw_cross(tag.cx(), tag.cy(), color = (0, 255, 0))
        RyDeg = degrees(tag.y_rotation())
        if (RyDeg > 180):
            RyDeg -= 360

        print_args = (int(tag.x_translation()), int(tag.z_translation()), int(RyDeg))
        #print_args = (tag.x_translation(), tag.z_translation(), RyDeg)

        if (abs(tag.x_translation()) > 1 and z_translation() > -4):
            if tag.x_translation() > 1:
                turnArg = (TurnSpeed, 1, int(x_translation() * 10))
                uart.write(("/turn/run %f %f %d\n" %turnArg).encode())
            elif tag.x_translation() < 1:
                turnArg = (TurnSpeed, -1, int(-x_translation() * 10))
                uart.write(("/turn/run %f %f %d\n" %turnArg).encode())
        elif (abs(tag.x_translation()) < 1 and z_translation() < -4):
            uart.write(("/straight/run %f\n" %StraightSpeed).encode())
        elif (RyDeg > AngleTH):
            rpcArg = (StraightSpeed, 1, int(RyDeg*RotateCoef), int(cos(tag.y_rotation()*4*StraightCoef)))
            uart.write(("/adjust/run %f %f %d %d\n" %rpcArg).encode())
        elif (RyDeg < -AngleTH):
            rpcArg = (StraightSpeed, -1, int(RyDeg*RotateCoef), int(-wcos(tag.y_rotation()*4*StraightCoef)))
            uart.write(("/adjust/run %f %f %d %d\n" %rpcArg).encode())
        else :
            uart.write(("/stop/run\n").encode())

        # Translation units are unknown. Rotation units are in degrees.
        #uart.write(("%d %d %d\n" %print_args).encode())
        print("%d %d %d\n" %print_args)
    #print(clock.fps())
