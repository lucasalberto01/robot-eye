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

    setCurrentSpeed(speedSettings::NORMAL);
}
// Move the car forward
void Car::moveForward(short int vel) {
    Serial.println("car is moving forward...");
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    digitalWrite(in3, LOW);
    digitalWrite(in4, HIGH);
    int pwm = map(vel, 0, 100, 0, 255);
    ledcWrite(channel_0, pwm);
    ledcWrite(channel_1, pwm);
}

// Move the car backward
void Car::moveBackward(short int vel) {
    Serial.println("car is moving backward...");
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    digitalWrite(in3, HIGH);
    digitalWrite(in4, LOW);

    int pwm = map(vel, 0, 100, 0, 255);
    ledcWrite(channel_0, pwm);
    ledcWrite(channel_1, pwm);
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

void Car::setCurrentSpeed(speedSettings speed) {
    currentSpeedSettings = speed;
}

int Car::checkLimit(int vel) {
    if (vel > 255) {
        return 255;
    } else if (vel < -255) {
        return -255;
    }

    return vel;
}

// Set the motor speed
void Car::turn(short int velL, short int velR) {
    Serial.println("car is turning...");
    int pwmL = map(velL, -100, 100, -255, 255);
    int pwmR = map(velR, -100, 100, -255, 255);

    pwmL = checkLimit(pwmL);
    pwmR = checkLimit(pwmR);

    if (pwmL >= 0) {
        digitalWrite(in1, LOW);
        digitalWrite(in2, HIGH);
    } else {
        digitalWrite(in1, HIGH);
        digitalWrite(in2, LOW);
    }

    if (pwmR >= 0) {
        digitalWrite(in3, LOW);
        digitalWrite(in4, HIGH);
    } else {
        digitalWrite(in3, HIGH);
        digitalWrite(in4, LOW);
    }

    ledcWrite(channel_0, abs(pwmL));
    ledcWrite(channel_1, abs(pwmR));
}

int Car::getSpeed() {
    return currentSpeedSettings;
}

void Car::turnLeft() {
    Serial.println("car is turning left...");
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    digitalWrite(in3, LOW);
    digitalWrite(in4, LOW);
    ledcWrite(channel_0, this->currentSpeedSettings);
    ledcWrite(channel_1, this->currentSpeedSettings);
}

void Car::turnRight() {
    Serial.println("car is turning right...");
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
    digitalWrite(in3, LOW);
    digitalWrite(in4, HIGH);
    ledcWrite(channel_0, this->currentSpeedSettings);
    ledcWrite(channel_1, this->currentSpeedSettings);
}

speedSettings Car::getCurrentSpeed() {
    return currentSpeedSettings;
}