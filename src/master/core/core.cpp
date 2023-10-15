#include "core.h"

#include <Arduino.h>

#include "../config.h"
#include "../core/state.h"
#include "../light/light.h"
#include "../motor/motor.h"
#include "../persona/persona.h"

Motor motor;
Persona persona;
Light light;

// Function to send commands to car
void sendCarCommand(const char* command) {
    // command could be either "left", "right", "forward" or "reverse" or "stop"
    // or speed settings "slow-speed", "normal-speed", or "fast-speed"
    // state_robot.lastCommandTime = millis();

    // Speed
    if (motor.processCommand(command) == 0) {
        return;
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