#include "car.h"

#include <Arduino.h>

Car::Car() {
    // Set all pins to output
    pinMode(in1, OUTPUT);
    pinMode(in2, OUTPUT);
    pinMode(in3, OUTPUT);
    pinMode(in4, OUTPUT);
    pinMode(SPEED_CONTROL_PIN_1, OUTPUT);
    pinMode(SPEED_CONTROL_PIN_2, OUTPUT);

    // Set initial motor state to OFF
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
    digitalWrite(in3, LOW);
    digitalWrite(in4, LOW);

    // Set the PWM Settings
    ledcSetup(channel_0, freq, resolution);
    ledcSetup(channel_1, freq, resolution);

    // Attach Pin to Channel
    ledcAttachPin(SPEED_CONTROL_PIN_1, channel_0);
    ledcAttachPin(SPEED_CONTROL_PIN_2, channel_1);

    // initialize default speed to SLOW
    setCurrentSpeed(speedSettings::NORMAL);
}

// Turn the car left
void Car::turnLeft() {
    Serial.println("car is turning left...");
    setMotorSpeed();
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    digitalWrite(in3, LOW);
    digitalWrite(in4, LOW);
}

// Turn the car right
void Car::turnRight() {
    Serial.println("car is turning right...");
    setMotorSpeed();
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
    digitalWrite(in3, LOW);
    digitalWrite(in4, HIGH);
}

// Move the car forward
void Car::moveForward() {
    Serial.println("car is moving forward...");
    setMotorSpeed();
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    digitalWrite(in3, LOW);
    digitalWrite(in4, HIGH);
}

// Move the car backward
void Car::moveBackward() {
    setMotorSpeed();
    Serial.println("car is moving backward...");
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    digitalWrite(in3, HIGH);
    digitalWrite(in4, LOW);
}

// Stop the car
void Car::stop() {
    Serial.println("car is stopping...");
    ledcWrite(channel_0, 0);
    ledcWrite(channel_1, 0);

    // // Turn off motors
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
    digitalWrite(in3, LOW);
    digitalWrite(in4, LOW);
}

// Set the motor speed
void Car::setMotorSpeed() {
    // change the duty cycle of the speed control pin connected to the motor
    Serial.print("Speed Settings: ");
    Serial.println(currentSpeedSettings);
    ledcWrite(channel_0, currentSpeedSettings);
    ledcWrite(channel_1, currentSpeedSettings);
}
// Set the current speed
void Car::setCurrentSpeed(speedSettings newSpeedSettings) {
    Serial.println("car is changing speed...");
    currentSpeedSettings = newSpeedSettings;
}
// Get the current speed
speedSettings Car::getCurrentSpeed() {
    return currentSpeedSettings;
}
