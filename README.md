# st17h66_FindMy
Firmware for Lenze ST17h66 that advertise to the Apple Find My network, based on SDK v3.1.1.2

## Compile
To compile this firmware CMake, arm-none-eabi-gcc and CMSIS headers are required. This is still very much a work in progress, mainly the linking is not working yet.
A instructions how to compile will follow when it is in a working state.

```
$ git clone https://github.com/ARM-software/CMSIS_5
$ git clone https://github.com/biemster/st17h66_FindMy
$ cd st17h66_FindMy/FindMy
$ mkdir build; cd build
$ cmake ..
$ make
```
