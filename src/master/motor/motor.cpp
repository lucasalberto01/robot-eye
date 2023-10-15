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

short Motor::processCommand(const char* command) {
    // Advance movement
    if (strstr(command, "motor#") != NULL) {
        strtok(strdup(command), "#");
        int angle = atoi(strtok(NULL, "#"));
        int speed = atoi(strtok(NULL, "#"));

        // Send command to car
        if ((angle > 80) && (angle < 100)) {
            moveForward(speed);
            return 0;
        } else if ((angle > 260) && (angle < 280)) {
            moveBackward(speed);
            return 0;
        } else if ((angle > 170) && (angle < 190)) {
            turn(0, speed);
            return 0;
        } else if ((angle > 350) || (angle < 10)) {
            turn(speed, 0);
            return 0;
        } else if ((angle > 100) && (angle < 170)) {
            turn(speed * 7 / 10, speed);
            return 0;
        } else if ((angle > 10) && (angle < 80)) {
            turn(speed, speed * 7 / 10);
            return 0;
        } else if ((angle > 190) && (angle < 260)) {
            turn(-1 * speed * 7 / 10, -1 * speed);
            return 0;
        } else if ((angle > 280) && (angle < 350)) {
            turn(-1 * speed, -1 * speed * 7 / 10);
            return 0;
        } else {
            stop();
            return 0;
        }

    }
    // Basic movement
    else if (strcmp(command, "left") == 0) {
        // persona.setAnimation(Persona::E_LOOK_R, true, false, true);
        turnLeft();
        return 0;
    } else if (strcmp(command, "right") == 0) {
        // persona.setAnimation(Persona::E_LOOK_L, true, false, true);
        turnRight();
        return 0;
    } else if (strcmp(command, "up") == 0) {
        moveForward(getSpeed());
        return 0;
    } else if (strcmp(command, "down") == 0) {
        moveBackward(getSpeed());
        return 0;
    } else if (strcmp(command, "stop") == 0) {
        stop();
        return 0;
    } else if (strcmp(command, "slow-speed") == 0) {
        setCurrentSpeed(speedSettings::SLOW);
        return 0;
    } else if (strcmp(command, "normal-speed") == 0) {
        setCurrentSpeed(speedSettings::NORMAL);
        return 0;
    } else if (strcmp(command, "fast-speed") == 0) {
        setCurrentSpeed(speedSettings::FAST);
        return 0;
    } else {
        return -1;
    }
}