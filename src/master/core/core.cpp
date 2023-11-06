#include "core.h"

#include <Arduino.h>



#include "../config.h"
#include "../core/state.h"
#include "../light/light.h"
#include "../motor/motor.h"
#include "../persona/persona.h"
#include "../control/control.h"

// Instantiate objects
Motor motor;
Persona persona;
Light light;
CamControl camControl;

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
        int dir = atoi(strtok(NULL, "#"));
        int speed = atoi(strtok(NULL, "#"));

        motor.setDirectionCaterpillar(dir, speed);

    }else if(strstr(command, "servo#") != NULL){
        strtok(strdup(command), "#");
        int tilt = atoi(strtok(NULL, "#"));
        int pan = atoi(strtok(NULL, "#"));

        Serial.printf("Tilt: %d, Pan: %d\n", tilt, pan);
        
        camControl.setCamPan(pan);
        camControl.setCamTilt(tilt);
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