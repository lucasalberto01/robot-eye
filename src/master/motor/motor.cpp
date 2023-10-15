#include "motor.h"

#include <Arduino.h>

#include "../config.h"

Motor::Motor() {
    // Set all pins to output
    pinMode(MOTOR_M1_IN1, OUTPUT);
    pinMode(MOTOR_M1_IN2, OUTPUT);
    pinMode(MOTOR_M2_IN1, OUTPUT);
    pinMode(MOTOR_M2_IN2, OUTPUT);
    pinMode(MOTOR_M1_EN, OUTPUT);
    pinMode(MOTOR_M2_EN, OUTPUT);

    // Set initial motor state to OFF
    digitalWrite(MOTOR_M1_IN1, LOW);
    digitalWrite(MOTOR_M1_IN2, LOW);
    digitalWrite(MOTOR_M2_IN1, LOW);
    digitalWrite(MOTOR_M2_IN2, LOW);

    // Set the PWM Settings
    ledcSetup(CHANNEL_0, FREQ, RESOLUTION);
    ledcSetup(CHANNEL_1, FREQ, RESOLUTION);

    // Attach Pin to Channel
    ledcAttachPin(MOTOR_M1_EN, CHANNEL_0);
    ledcAttachPin(MOTOR_M2_EN, CHANNEL_1);

    setCurrentSpeed(speedSettings::NORMAL);
}
// Move the car forward
void Motor::moveForward(short int vel) {
    digitalWrite(MOTOR_M1_IN1, LOW);
    digitalWrite(MOTOR_M1_IN2, HIGH);
    digitalWrite(MOTOR_M2_IN1, LOW);
    digitalWrite(MOTOR_M2_IN2, HIGH);
    int pwm = map(vel, 0, 100, 0, 255);
    ledcWrite(CHANNEL_0, pwm);
    ledcWrite(CHANNEL_1, pwm);
}

// Move the car backward
void Motor::moveBackward(short int vel) {
    digitalWrite(MOTOR_M1_IN1, HIGH);
    digitalWrite(MOTOR_M1_IN2, LOW);
    digitalWrite(MOTOR_M2_IN1, HIGH);
    digitalWrite(MOTOR_M2_IN2, LOW);

    int pwm = map(vel, 0, 100, 0, 255);
    ledcWrite(CHANNEL_0, pwm);
    ledcWrite(CHANNEL_1, pwm);
}

// Stop the car
void Motor::stop() {
    ledcWrite(CHANNEL_0, 0);
    ledcWrite(CHANNEL_1, 0);

    // // Turn off motors
    digitalWrite(MOTOR_M1_IN1, LOW);
    digitalWrite(MOTOR_M1_IN2, LOW);
    digitalWrite(MOTOR_M2_IN1, LOW);
    digitalWrite(MOTOR_M2_IN2, LOW);
}

void Motor::setCurrentSpeed(speedSettings speed) {
    currentSpeedSettings = speed;
}

int Motor::checkLimit(int vel) {
    if (vel > 255) {
        return 255;
    } else if (vel < -255) {
        return -255;
    }

    return vel;
}

// Set the motor speed
void Motor::turn(short int velL, short int velR) {
    int pwmL = map(velL, -100, 100, -255, 255);
    int pwmR = map(velR, -100, 100, -255, 255);

    pwmL = checkLimit(pwmL);
    pwmR = checkLimit(pwmR);

    if (pwmL >= 0) {
        digitalWrite(MOTOR_M1_IN1, LOW);
        digitalWrite(MOTOR_M1_IN2, HIGH);
    } else {
        digitalWrite(MOTOR_M1_IN1, HIGH);
        digitalWrite(MOTOR_M1_IN2, LOW);
    }

    if (pwmR >= 0) {
        digitalWrite(MOTOR_M2_IN1, LOW);
        digitalWrite(MOTOR_M2_IN2, HIGH);
    } else {
        digitalWrite(MOTOR_M2_IN1, HIGH);
        digitalWrite(MOTOR_M2_IN2, LOW);
    }

    ledcWrite(CHANNEL_0, abs(pwmL));
    ledcWrite(CHANNEL_1, abs(pwmR));
}

int Motor::getSpeed() {
    return currentSpeedSettings;
}

void Motor::turnLeft() {
    digitalWrite(MOTOR_M1_IN1, LOW);
    digitalWrite(MOTOR_M1_IN2, HIGH);
    digitalWrite(MOTOR_M2_IN1, LOW);
    digitalWrite(MOTOR_M2_IN2, LOW);
    ledcWrite(CHANNEL_0, this->currentSpeedSettings);
    ledcWrite(CHANNEL_1, this->currentSpeedSettings);
}

void Motor::turnRight() {
    digitalWrite(MOTOR_M1_IN1, LOW);
    digitalWrite(MOTOR_M1_IN2, LOW);
    digitalWrite(MOTOR_M2_IN1, LOW);
    digitalWrite(MOTOR_M2_IN2, HIGH);
    ledcWrite(CHANNEL_0, this->currentSpeedSettings);
    ledcWrite(CHANNEL_1, this->currentSpeedSettings);
}

speedSettings Motor::getCurrentSpeed() {
    return currentSpeedSettings;
}
