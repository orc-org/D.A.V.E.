# D.A.V.E.
Introducing: D.A.V.E.! Okanagan Rover Craft's First Rover developed using the ROS2 Humble Hawksbill Framework! :) :)


Pinout

Adafruit IMU: Pins 3, 5

XLR: Pins, 8, 10

Stepper Arm: Pins 15, 7, 11

Stepper Hand: Pins 32, 31, 29

Lights: Pin 40

Servo: 19, 21, 23, 24, 26, 27


# _Code to run morse servo_ (note: It uses pin 32 on the jetson nano)
 1. ``` g++ jetson_morse.cpp -o jetson_morse -pthread```
 2. ``` sudo ./jetson_moore ```