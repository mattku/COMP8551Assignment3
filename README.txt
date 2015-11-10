Comp8551 Assignment 3

Brian Thomas
Matt Ku

This application should be run on OSX or linux.

*******BUILDING THE CODE*******
  <from the project root>
  ./build.sh

*******RUNNING THE APP*******
  <from the project root>
  ./run.sh

*******OUTPUT*******

  first, you should see convolution of a 3x3 blur filter on a 16x16 image. The source image's and resulting images' R channels are displayed for comparison.
  then, you should see the timings of a 15x15 filter on a 4Kx4K image.
  On a lab computer, the total runtime was ~40 seconds.
  (gpu only: 4s, cpu only: 5s, combined: 3.5s, serial: 25s)

  note: the cpu/gpu split runs convolution on one half each on the cpu, gpu.
  We did not recombine the image into one result, for convenience. It would be faster to do two smaller cl image reads and copy the data from each into the same Image.
