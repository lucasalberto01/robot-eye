#ifndef MOTOR_H
#define MOTOR_H

#include "../config.h"
#include "../types.h"

/*
  The resolution of the PWM is 8 bit so the value is between 0-255
  We will set the speed between 100 to 255.
*/

// EN = is the Arduino pin (requires a PWM pin) connected to the Enable pin of the module
// IN1 and IN2 are the two digital pins connected to IN1 and IN2 pins of the module

// PWM Setup to control motor speed
// Do not change these values
#define FREQ 2000
#define CHANNEL_0 1
#define CHANNEL_1 2
#define RESOLUTION 8

class Motor {
   private:
    // Check if the speed is within the limit
    int checkLimit(int vel);

   protected:
    // holds the current speed settings, see values for SLOW, NORMAL, FAST
    speedSettings currentSpeedSettings = speedSettings::NORMAL;

   public:
    Motor();
    // Move the car forward
    void moveForward(short int vel);
    // Move the car backward
    void moveBackward(short int vel);
    // Stop the car
    void stop();
    // Set the speed settings
    void setCurrentSpeed(speedSettings speed);
    // Set the motor speed
    void turn(short int velLeft, short int velRight);
    // Get the current speed settings
    int getSpeed();
    // Turn the car left
    void turnLeft();
    // Turn the car right
    void turnRight();
    // Get the current speed settings
    speedSettings getCurrentSpeed();
};

#endif