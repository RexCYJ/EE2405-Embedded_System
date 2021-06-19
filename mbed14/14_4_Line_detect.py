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

#uart = pyb.UART(3,9600,timeout_char=1000)
#uart.init(9600,bits=8,parity = None, stop=1, timeout_char=1000)



while(True):
   clock.tick()
   img = sensor.snapshot().histeq()
   img.binary([(240,255)])
   img.erode(1)
   if enable_lens_corr: img.lens_corr(1.8) # for 2.8mm lens...

   # `merge_distance` controls the merging of nearby lines. At 0 (the default), no
   # merging is done. At 1, any line 1 pixel away from another is merged... and so
   # on as you increase this value. You may wish to merge lines as line segment
   # detection produces a lot of line segment results.

   # `max_theta_diff` controls the maximum amount of rotation difference between
   # any two lines about to be merged. The default setting allows for 15 degrees.

   line = img.get_regression([(255, 255)], robust=True)
   print(line)


   #for l in img.find_line_segments(merge_distance = 1, max_theta_diff = 3):
      ##if l.y1() < 80 and l.y2() < 80 and (l.x1() > 25 or l.x2()> 25) and (l.x1() < 140 or l.x2() < 140) and l.magnitude() > 10 and l.length() > 20:
        #print(l)
   img.draw_line(line.line(), color=127)

        #print_args = (l.x1(), l.x2(), l.y1(), l.y2())
        #uart.write(("%d %d %d %d" %print_args).encode())
        #print("%d %d %d %d" %print_args)
   #print("FPS %f" % clock.fps())
