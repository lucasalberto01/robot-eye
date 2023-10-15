#include "core.h"

#include <Arduino.h>

#include "../config.h"
#include "../core/state.h"
#include "../light/light.h"
#include "../motor/motor.h"
#include "../persona/persona.h"

// Instantiate objects
Motor motor;
Persona persona;
Light light;

// State
extern TStateRobot state_robot;

// Function to send commands to motor
void sendCarCommand(const char* command) {
    // command could be either "left", "right", "forward" or "reverse" or "stop"
    // or speed settings "slow-speed", "normal-speed", or "fast-speed"
    state_robot.lastCommandTime = millis();

    // Advance movement
    if (strstr(command, "motor#") != NULL) {
        strtok(strdup(command), "#");
        int angle = atoi(strtok(NULL, "#"));
        int speed = atoi(strtok(NULL, "#"));

        // Send command to car
        if ((angle > 80) && (angle < 100)) {
            motor.moveForward(speed);
        } else if ((angle > 260) && (angle < 280)) {
            motor.moveBackward(speed);
        } else if ((angle > 170) && (angle < 190)) {
            motor.turn(0, speed);
        } else if ((angle > 350) || (angle < 10)) {
            motor.turn(speed, 0);
        } else if ((angle > 100) && (angle < 170)) {
            motor.turn(speed * 7 / 10, speed);
        } else if ((angle > 10) && (angle < 80)) {
            motor.turn(speed, speed * 7 / 10);
        } else if ((angle > 190) && (angle < 260)) {
            motor.turn(-1 * speed * 7 / 10, -1 * speed);
        } else if ((angle > 280) && (angle < 350)) {
            motor.turn(-1 * speed, -1 * speed * 7 / 10);
        } else {
            motor.stop();
        }

    }
    // Basic movement
    else if (strcmp(command, "left") == 0) {
        persona.setAnimation(Persona::E_LOOK_R, true, false, true);
        motor.turnLeft();
    } else if (strcmp(command, "right") == 0) {
        persona.setAnimation(Persona::E_LOOK_L, true, false, true);
        motor.turnRight();
    } else if (strcmp(command, "up") == 0) {
        motor.moveForward(motor.getSpeed());
    } else if (strcmp(command, "down") == 0) {
        motor.moveBackward(motor.getSpeed());
    } else if (strcmp(command, "stop") == 0) {
        motor.stop();
    } else if (strcmp(command, "slow-speed") == 0) {
        motor.setCurrentSpeed(speedSettings::SLOW);
    } else if (strcmp(command, "normal-speed") == 0) {
        motor.setCurrentSpeed(speedSettings::NORMAL);
    } else if (strcmp(command, "fast-speed") == 0) {
        motor.setCurrentSpeed(speedSettings::FAST);
    }
    // Light
    else if (strcmp(command, "led") == 0) {
        light.toggle();
    }
    // Emotions
    else {
        persona.processCommand(command);
    }
}