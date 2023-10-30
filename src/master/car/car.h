#ifndef CAR_H
#define CAR_H

/*
  The resolution of the PWM is 8 bit so the value is between 0-255
  We will set the speed between 100 to 255.
*/
enum speedSettings {
    SLOW = 165,
    NORMAL = 185,
    FAST = 255
};

class Car {
   private:
    // Motor 1 connections
    int in1 = 0;
    int in2 = 4;
    // Motor 2 connections
    int in3 = 32;
    int in4 = 33;

    // PWM Setup to control motor speed
    const int SPEED_CONTROL_PIN_1 = 25;
    const int SPEED_CONTROL_PIN_2 = 26;
    // Play around with the frequency settings depending on the motor that you are using
    const int freq = 2000;
    const int channel_0 = 1;
    const int channel_1 = 2;
    // 8 Bit resolution for duty cycle so value is between 0 - 255
    const int resolution = 8;

   protected:
    // holds the current speed settings, see values for SLOW, NORMAL, FAST
    speedSettings currentSpeedSettings;

   public:
    Car();
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
    // Check if the speed is within the limit
    int checkLimit(int vel);
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